/*
 * NetLang Code Generator Implementation
 * 
 * Generates high-performance C code for CNN inference.
 */

#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ========== HELPER FUNCTIONS ========== */

static void emit(FILE* out, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
}

void emit_comment_separator(FILE* out, const char* title) {
    if (!title) title = "UNKNOWN";
    emit(out, "\n    /* ========== %s ========== */\n", title);
}

static int calculate_tensor_size(TensorType* shape) {
    if (!shape) return 0;
    int size = 1;
    for (int i = 0; i < shape->rank; i++) {
        size *= shape->dims[i];
    }
    return size;
}

/* Recompute output shape for a layer (needed because symbol table only has final shape) */
static TensorType* compute_layer_output_shape(LayerInfo* layer) {
    if (!layer->input_shape) return NULL;
    
    TensorType* output = calloc(1, sizeof(TensorType));
    TensorType* input = layer->input_shape;
    
    switch (layer->layer_type) {
        case NODE_CONV2D: {
            ASTNode* node = layer->layer_node;
            int filters = node->data.conv2d.filters;
            int kernel_h = node->data.conv2d.kernel[0];
            int kernel_w = node->data.conv2d.kernel[1];
            int stride = node->data.conv2d.stride;
            int padding = node->data.conv2d.padding;
            
            output->rank = 3;
            output->dims[0] = (input->dims[0] + 2*padding - kernel_h) / stride + 1;
            output->dims[1] = (input->dims[1] + 2*padding - kernel_w) / stride + 1;
            output->dims[2] = filters;
            break;
        }
        case NODE_MAXPOOL:
        case NODE_AVGPOOL: {
            ASTNode* node = layer->layer_node;
            int pool_h = node->data.pooling.pool[0];
            int pool_w = node->data.pooling.pool[1];
            int stride = node->data.pooling.stride;
            int padding = node->data.pooling.padding;
            
            if (stride == 0) stride = pool_h;  // Default stride
            
            output->rank = 3;
            output->dims[0] = (input->dims[0] + 2*padding - pool_h) / stride + 1;
            output->dims[1] = (input->dims[1] + 2*padding - pool_w) / stride + 1;
            output->dims[2] = input->dims[2];  // Channels unchanged
            break;
        }
        case NODE_DENSE: {
            ASTNode* node = layer->layer_node;
            output->rank = 1;
            output->dims[0] = node->data.dense.units;
            break;
        }
        case NODE_FLATTEN: {
            output->rank = 1;
            output->dims[0] = calculate_tensor_size(input);
            break;
        }
        default:
            free(output);
            return NULL;
    }
    
    output->dtype = "float32";
    return output;
}

void emit_tensor_allocation(FILE* out, const char* name, TensorType* shape) {
    int size = calculate_tensor_size(shape);
    emit(out, "    float* %s = aligned_alloc_64(%d * sizeof(float));\n", name, size);
    
    /* Emit shape information as comment for debugging */
    emit(out, "    /* Shape: [");
    for (int i = 0; i < shape->rank; i++) {
        emit(out, "%d", shape->dims[i]);
        if (i < shape->rank - 1) emit(out, ", ");
    }
    emit(out, "] = %d elements */\n", size);
}

/* ========== LAYER EXTRACTION ========== */

void extract_layers(CodegenContext* ctx, ASTNode* network) {
    if (!network || network->type != NODE_NETWORK) return;
    
    /* Count layers and allocate array */
    ASTList* stmts = network->data.network.statements;
    ctx->layer_count = stmts->count;
    ctx->layers = calloc(ctx->layer_count, sizeof(LayerInfo));
    
    ctx->weight_layer_count = 0;
    int idx = 0;
    ASTNode* stmt = stmts->head;
    
    while (stmt) {
        if (stmt->type == NODE_ASSIGNMENT) {
            LayerInfo* layer = &ctx->layers[idx];
            
            /* Create unique variable name for this layer */
            char unique_name[64];
            snprintf(unique_name, sizeof(unique_name), "layer_%d", idx);
            layer->var_name = strdup(unique_name);
            
            /* Get layer node (unwrap FROM_EXPR if present) */
            ASTNode* layer_node = stmt->data.assignment.value;
            if (layer_node->type == NODE_FROM_EXPR) {
                layer_node = layer_node->data.from_expr.layer;
            }
            
            layer->layer_type = layer_node->type;
            layer->layer_node = layer_node;
            
            /* Get input shape: for first layer use network input, else use previous layer's output */
            if (idx == 0) {
                /* First layer - use network input */
                Symbol* input_sym = scope_lookup(ctx->scope, "input", 1);
                if (input_sym && input_sym->tensor_type) {
                    layer->input_shape = input_sym->tensor_type;
                }
            } else {
                /* Subsequent layers - use previous layer's output as input */
                layer->input_shape = ctx->layers[idx - 1].output_shape;
            }
            
            /* Compute output shape based on layer type and input shape */
            layer->output_shape = compute_layer_output_shape(layer);
            
            /* Assign layer index for weight-bearing layers */
            if (layer->layer_type == NODE_CONV2D || layer->layer_type == NODE_DENSE) {
                layer->layer_index = ctx->weight_layer_count++;
            } else {
                layer->layer_index = -1;
            }
            
            idx++;
        }
        stmt = stmt->next;
    }
}

/* ========== FUSION DETECTION ========== */

void detect_fusion_opportunities(CodegenContext* ctx) {
    for (int i = 0; i < ctx->layer_count - 1; i++) {
        LayerInfo* curr = &ctx->layers[i];
        
        /* Check if Conv2D or Dense has ReLU activation */
        if (curr->layer_type == NODE_CONV2D) {
            ActivationType act = curr->layer_node->data.conv2d.activation;
            if (act == ACT_RELU) {
                curr->can_fuse_relu = 1;
            }
        } else if (curr->layer_type == NODE_DENSE) {
            ActivationType act = curr->layer_node->data.dense.activation;
            if (act == ACT_RELU) {
                curr->can_fuse_relu = 1;
            }
        }
    }
}

/* ========== CODE EMISSION: HEADERS AND STRUCTURE ========== */

void emit_includes(CodegenContext* ctx) {
    emit(ctx->output, "/*\n");
    emit(ctx->output, " * Generated by NetLang Compiler\n");
    emit(ctx->output, " * Network: %s\n", ctx->network_name);
    emit(ctx->output, " * Optimizations: Shape specialization, Aligned memory, Operator fusion\n");
    emit(ctx->output, " * Compile: gcc -O3 -march=haswell -mavx2 -mfma -o network network.c\n");
    emit(ctx->output, " */\n\n");
    
    emit(ctx->output, "#include <stdio.h>\n");
    emit(ctx->output, "#include <stdlib.h>\n");
    emit(ctx->output, "#include <string.h>\n");
    emit(ctx->output, "#include <stdint.h>\n");
    emit(ctx->output, "#ifdef _WIN32\n");
    emit(ctx->output, "#include <malloc.h>\n");
    emit(ctx->output, "#endif\n");
    emit(ctx->output, "#include \"src/codegen/runtime.h\"\n");
    emit(ctx->output, "#include \"src/codegen/kernels.h\"\n\n");
}

void emit_network_struct(CodegenContext* ctx) {
    emit(ctx->output, "/* Network structure */\n");
    emit(ctx->output, "typedef struct {\n");
    emit(ctx->output, "    WeightFile* weights;\n");
    
    /* Allocate tensors for all intermediate layers */
    for (int i = 0; i < ctx->layer_count; i++) {
        LayerInfo* layer = &ctx->layers[i];
        emit(ctx->output, "    float* %s;  /* ", layer->var_name);
        if (layer->output_shape) {
            emit(ctx->output, "Shape: [");
            for (int j = 0; j < layer->output_shape->rank; j++) {
                emit(ctx->output, "%d", layer->output_shape->dims[j]);
                if (j < layer->output_shape->rank - 1) emit(ctx->output, ", ");
            }
            emit(ctx->output, "]");
        }
        emit(ctx->output, " */\n");
    }
    
    emit(ctx->output, "} NetworkState;\n\n");
}

/* ========== INITIALIZATION ========== */

void emit_init_function(CodegenContext* ctx) {
    emit(ctx->output, "/* Initialize network: load weights and allocate tensors */\n");
    emit(ctx->output, "NetworkState* network_init() {\n");
    emit(ctx->output, "    NetworkState* net = calloc(1, sizeof(NetworkState));\n");
    emit(ctx->output, "    if (!net) {\n");
    emit(ctx->output, "        fprintf(stderr, \"Failed to allocate network state\\n\");\n");
    emit(ctx->output, "        return NULL;\n");
    emit(ctx->output, "    }\n\n");
    
    /* Load weights */
    emit_comment_separator(ctx->output, "Load Weights");
    emit(ctx->output, "    net->weights = load_weights(\"%s\");\n", ctx->weight_path);
    emit(ctx->output, "    if (!net->weights) {\n");
    emit(ctx->output, "        fprintf(stderr, \"Failed to load weights from %s\\n\");\n", ctx->weight_path);
    emit(ctx->output, "        free(net);\n");
    emit(ctx->output, "        return NULL;\n");
    emit(ctx->output, "    }\n\n");
    
    /* Allocate tensors */
    emit_comment_separator(ctx->output, "Allocate Tensors");
    for (int i = 0; i < ctx->layer_count; i++) {
        LayerInfo* layer = &ctx->layers[i];
        if (layer->output_shape) {
            int size = calculate_tensor_size(layer->output_shape);
            emit(ctx->output, "    net->%s = aligned_alloc_64(%d * sizeof(float));\n", 
                 layer->var_name, size);
        }
    }
    
    emit(ctx->output, "\n    return net;\n");
    emit(ctx->output, "}\n\n");
}

/* ========== INFERENCE FUNCTION ========== */

void emit_infer_function(CodegenContext* ctx) {
    emit(ctx->output, "/* Run inference: input -> output */\n");
    emit(ctx->output, "void network_infer(NetworkState* net, float* input, float* output) {\n");
    
    /* Generate layer-by-layer code */
    for (int i = 0; i < ctx->layer_count; i++) {
        LayerInfo* layer = &ctx->layers[i];
        emit_comment_separator(ctx->output, layer->var_name);
        emit_layer_code(ctx, layer, i);
        emit(ctx->output, "\n");
    }
    
    /* Copy final output */
    LayerInfo* last_layer = &ctx->layers[ctx->layer_count - 1];
    int output_size = calculate_tensor_size(last_layer->output_shape);
    emit(ctx->output, "    /* Copy output */\n");
    emit(ctx->output, "    memcpy(output, net->%s, %d * sizeof(float));\n", 
         last_layer->var_name, output_size);
    
    emit(ctx->output, "}\n\n");
}

/* ========== LAYER CODE EMISSION ========== */

void emit_layer_code(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    switch (layer->layer_type) {
        case NODE_CONV2D:
            emit_conv2d(ctx, layer, layer_idx);
            break;
        case NODE_DENSE:
            emit_dense(ctx, layer, layer_idx);
            break;
        case NODE_MAXPOOL:
        case NODE_AVGPOOL:
            emit_pooling(ctx, layer, layer_idx);
            break;
        case NODE_FLATTEN:
            emit_flatten(ctx, layer, layer_idx);
            break;
        default:
            emit(ctx->output, "    /* TODO: Layer type %d */\n", layer->layer_type);
    }
}

void emit_conv2d(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    ASTNode* node = layer->layer_node;
    int filters = node->data.conv2d.filters;
    int kernel_h = node->data.conv2d.kernel[0];
    int kernel_w = node->data.conv2d.kernel[1];
    int stride = node->data.conv2d.stride;
    int padding = node->data.conv2d.padding;
    ActivationType act = node->data.conv2d.activation;
    
    /* Get input tensor name (previous layer or "input") */
    char input_name[64];
    if (layer_idx == 0) {
        snprintf(input_name, sizeof(input_name), "input");
    } else {
        snprintf(input_name, sizeof(input_name), "net->%s", ctx->layers[layer_idx - 1].var_name);
    }
    
    /* Get shapes for specialized code */
    TensorType* in_shape = layer->input_shape;
    TensorType* out_shape = layer->output_shape;
    
    if (!in_shape || !out_shape) {
        emit(ctx->output, "    /* ERROR: Missing shape information */\n");
        return;
    }
    
    /* Extract dimensions (CNN format: [H, W, C] or [B, H, W, C]) */
    int in_h = in_shape->dims[0];
    int in_w = in_shape->dims[1];
    int in_c = in_shape->dims[2];
    int out_h = out_shape->dims[0];
    int out_w = out_shape->dims[1];
    
    /* Get weights and bias from .nwf file */
    emit(ctx->output, "    const float* conv_weights_%d = get_layer_weights(net->weights, %d);\n", 
         layer_idx, layer->layer_index);
    emit(ctx->output, "    const float* conv_bias_%d = get_layer_bias(net->weights, %d);\n", 
         layer_idx, layer->layer_index);
    
    /* Emit specialized Conv2D call with compile-time constants */
    emit(ctx->output, "    /* Conv2D: [%d,%d,%d] -> [%d,%d,%d], K=%dx%d, S=%d, P=%d */\n",
         in_h, in_w, in_c, out_h, out_w, filters, kernel_h, kernel_w, stride, padding);
    
    if (layer->can_fuse_relu && act == ACT_RELU) {
        emit(ctx->output, "    conv2d_relu_forward_avx2(\n");
    } else {
        emit(ctx->output, "    conv2d_forward_avx2(\n");
    }
    
    emit(ctx->output, "        %s, conv_weights_%d, conv_bias_%d, net->%s,\n", 
         input_name, layer_idx, layer_idx, layer->var_name);
    emit(ctx->output, "        %d, %d, %d,  /* in: H, W, C */\n", in_h, in_w, in_c);
    emit(ctx->output, "        %d, %d,      /* kernel: H, W */\n", kernel_h, kernel_w);
    emit(ctx->output, "        %d,          /* out channels */\n", filters);
    emit(ctx->output, "        %d, %d       /* stride, padding */\n", stride, padding);
    emit(ctx->output, "    );\n");
    
    /* Apply activation if not fused */
    if (!layer->can_fuse_relu && act != ACT_NONE && act != ACT_LINEAR) {
        emit_activation(ctx, layer, act);
    }
}

void emit_dense(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    ASTNode* node = layer->layer_node;
    
    if (!node) {
        emit(ctx->output, "    /* ERROR: NULL layer node */\n");
        return;
    }
    
    int units = node->data.dense.units;
    ActivationType act = node->data.dense.activation;
    
    /* Get input tensor name */
    char input_name[64];
    if (layer_idx == 0) {
        snprintf(input_name, sizeof(input_name), "input");
    } else {
        snprintf(input_name, sizeof(input_name), "net->%s", ctx->layers[layer_idx - 1].var_name);
    }
    
    /* Get shapes */
    TensorType* in_shape = layer->input_shape;
    
    if (!in_shape) {
        emit(ctx->output, "    /* ERROR: Missing input shape for Dense */\n");
        return;
    }
    
    int in_features = calculate_tensor_size(in_shape);
    
    /* Get weights and bias */
    emit(ctx->output, "    const float* dense_weights_%d = get_layer_weights(net->weights, %d);\n", 
         layer_idx, layer->layer_index);
    emit(ctx->output, "    const float* dense_bias_%d = get_layer_bias(net->weights, %d);\n", 
         layer_idx, layer->layer_index);
    
    emit(ctx->output, "    /* Dense: %d -> %d */\n", in_features, units);
    
    if (layer->can_fuse_relu && act == ACT_RELU) {
        emit(ctx->output, "    dense_relu_forward_avx2(\n");
    } else {
        emit(ctx->output, "    dense_forward_avx2(\n");
    }
    
    emit(ctx->output, "        %s, dense_weights_%d, dense_bias_%d, net->%s,\n",
         input_name, layer_idx, layer_idx, layer->var_name);
    emit(ctx->output, "        %d, %d  /* in_features, out_features */\n", in_features, units);
    emit(ctx->output, "    );\n");
    
    /* Apply activation if not fused */
    if (!layer->can_fuse_relu && act != ACT_NONE && act != ACT_LINEAR) {
        emit_activation(ctx, layer, act);
    }
}

void emit_pooling(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    ASTNode* node = layer->layer_node;
    
    if (!node) {
        emit(ctx->output, "    /* ERROR: NULL layer node */\n");
        return;
    }
    
    int pool_h = node->data.pooling.pool[0];
    int pool_w = node->data.pooling.pool[1];
    int stride = node->data.pooling.stride;
    int padding = node->data.pooling.padding;
    
    char input_name[64];
    if (layer_idx == 0) {
        snprintf(input_name, sizeof(input_name), "input");
    } else {
        snprintf(input_name, sizeof(input_name), "net->%s", ctx->layers[layer_idx - 1].var_name);
    }
    
    TensorType* in_shape = layer->input_shape;
    TensorType* out_shape = layer->output_shape;
    
    if (!in_shape || !out_shape) {
        emit(ctx->output, "    /* ERROR: Missing shape information for pooling */\n");
        return;
    }
    
    int in_h = in_shape->dims[0];
    int in_w = in_shape->dims[1];
    int channels = in_shape->dims[2];
    int out_h = out_shape->dims[0];
    int out_w = out_shape->dims[1];
    
    const char* pool_func = (layer->layer_type == NODE_MAXPOOL) ? "maxpool2d_forward" : "avgpool2d_forward";
    
    emit(ctx->output, "    /* %s: [%d,%d,%d] -> [%d,%d,%d], P=%dx%d, S=%d */\n",
         (layer->layer_type == NODE_MAXPOOL) ? "MaxPool" : "AvgPool",
         in_h, in_w, channels, out_h, out_w, channels, pool_h, pool_w, stride);
    emit(ctx->output, "    %s(\n", pool_func);
    emit(ctx->output, "        %s, net->%s,\n", input_name, layer->var_name);
    emit(ctx->output, "        %d, %d, %d,  /* in: H, W, C */\n", in_h, in_w, channels);
    emit(ctx->output, "        %d, %d,      /* pool: H, W */\n", pool_h, pool_w);
    emit(ctx->output, "        %d, %d       /* stride, padding */\n", stride, padding);
    emit(ctx->output, "    );\n");
}

void emit_flatten(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    char input_name[64];
    if (layer_idx == 0) {
        snprintf(input_name, sizeof(input_name), "input");
    } else {
        snprintf(input_name, sizeof(input_name), "net->%s", ctx->layers[layer_idx - 1].var_name);
    }
    
    if (!layer->input_shape) {
        emit(ctx->output, "    /* ERROR: Missing input shape for flatten */\n");
        return;
    }
    
    int size = calculate_tensor_size(layer->input_shape);
    
    emit(ctx->output, "    /* Flatten: %d elements (no-op, just pointer) */\n", size);
    emit(ctx->output, "    net->%s = %s;\n", layer->var_name, input_name);
}

void emit_activation(CodegenContext* ctx, LayerInfo* layer, ActivationType act) {
    int size = calculate_tensor_size(layer->output_shape);
    
    switch (act) {
        case ACT_RELU:
            emit(ctx->output, "    relu_inplace_avx2(net->%s, %d);\n", layer->var_name, size);
            break;
        case ACT_SIGMOID:
            emit(ctx->output, "    sigmoid_inplace_avx2(net->%s, %d);\n", layer->var_name, size);
            break;
        case ACT_TANH:
            emit(ctx->output, "    tanh_inplace_avx2(net->%s, %d);\n", layer->var_name, size);
            break;
        case ACT_SOFTMAX:
            emit(ctx->output, "    softmax_inplace(net->%s, %d);\n", layer->var_name, size);
            break;
        default:
            break;
    }
}

/* ========== CLEANUP ========== */

void emit_cleanup_function(CodegenContext* ctx) {
    emit(ctx->output, "/* Cleanup: free tensors and weights */\n");
    emit(ctx->output, "void network_cleanup(NetworkState* net) {\n");
    emit(ctx->output, "    if (!net) return;\n\n");
    
    /* Free tensors */
    for (int i = 0; i < ctx->layer_count; i++) {
        LayerInfo* layer = &ctx->layers[i];
        /* Skip flatten layers (they alias other tensors) */
        if (layer->layer_type != NODE_FLATTEN) {
            emit(ctx->output, "    aligned_free(net->%s);\n", layer->var_name);
        }
    }
    
    emit(ctx->output, "    unload_weights(net->weights);\n");
    emit(ctx->output, "    free(net);\n");
    emit(ctx->output, "}\n\n");
}

/* ========== MAIN FUNCTION ========== */

void emit_main_function(CodegenContext* ctx) {
    emit(ctx->output, "/* Main: simple test harness */\n");
    emit(ctx->output, "int main() {\n");
    emit(ctx->output, "    printf(\"Initializing network: %s\\n\");\n", ctx->network_name);
    emit(ctx->output, "    NetworkState* net = network_init();\n");
    emit(ctx->output, "    if (!net) return 1;\n\n");
    
    /* Allocate input and output */
    int input_size = calculate_tensor_size(ctx->input_shape);
    LayerInfo* last_layer = &ctx->layers[ctx->layer_count - 1];
    int output_size = calculate_tensor_size(last_layer->output_shape);
    
    emit(ctx->output, "    /* Allocate input and output */\n");
    emit(ctx->output, "    float* input = aligned_alloc_64(%d * sizeof(float));\n", input_size);
    emit(ctx->output, "    float* output = aligned_alloc_64(%d * sizeof(float));\n", output_size);
    emit(ctx->output, "\n");
    
    /* Initialize test input (zeros for now) */
    emit(ctx->output, "    /* Initialize test input */\n");
    emit(ctx->output, "    memset(input, 0, %d * sizeof(float));\n\n", input_size);
    
    /* Run inference */
    emit(ctx->output, "    printf(\"Running inference...\\n\");\n");
    emit(ctx->output, "    network_infer(net, input, output);\n\n");
    
    /* Print results */
    emit(ctx->output, "    printf(\"Output: \");\n");
    emit(ctx->output, "    for (int i = 0; i < %d && i < 10; i++) {\n", output_size);
    emit(ctx->output, "        printf(\"%%f \", output[i]);\n");
    emit(ctx->output, "    }\n");
    emit(ctx->output, "    printf(\"...\\n\");\n\n");
    
    /* Cleanup */
    emit(ctx->output, "    aligned_free(input);\n");
    emit(ctx->output, "    aligned_free(output);\n");
    emit(ctx->output, "    network_cleanup(net);\n");
    emit(ctx->output, "    printf(\"Done!\\n\");\n");
    emit(ctx->output, "    return 0;\n");
    emit(ctx->output, "}\n");
}

/* ========== MAIN ENTRY POINT ========== */

void generate_network_code(ASTNode* network, Scope* global_scope, FILE* output) {
    if (!network || network->type != NODE_NETWORK) {
        fprintf(stderr, "ERROR: Invalid network node\n");
        return;
    }
    
    /* Find the network's scope (it's a child of global_scope) */
    char scope_name[256];
    snprintf(scope_name, sizeof(scope_name), "network:%s", network->data.network.name);
    
    Scope* network_scope = NULL;
    if (global_scope && global_scope->children) {
        Scope* child = global_scope->children;
        while (child) {
            if (child->name && strcmp(child->name, scope_name) == 0) {
                network_scope = child;
                break;
            }
            child = child->next;
        }
    }
    
    if (!network_scope) {
        fprintf(stderr, "ERROR: Cannot find network scope '%s'\n", scope_name);
        network_scope = global_scope;  /* Fallback */
    }
    
    /* Initialize context */
    CodegenContext ctx = {0};
    ctx.output = output;
    ctx.scope = network_scope;  /* Use network scope, not global */
    ctx.network_name = network->data.network.name;
    
    /* Get weight path */
    if (network->data.network.weights) {
        ctx.weight_path = network->data.network.weights->data.weights.path;
    } else {
        ctx.weight_path = "weights.nwf";  /* Default */
    }
    
    /* Get input shape from network scope */
    if (network->data.network.input) {
        Symbol* input_sym = scope_lookup(network_scope, "input", 0);  /* Don't recurse, look in THIS scope */
        if (input_sym && input_sym->tensor_type) {
            ctx.input_shape = input_sym->tensor_type;
        }
    }
    
    /* Extract network structure */
    extract_layers(&ctx, network);
    detect_fusion_opportunities(&ctx);
    
    /* Generate code */
    emit_includes(&ctx);
    emit_network_struct(&ctx);
    emit_init_function(&ctx);
    emit_infer_function(&ctx);
    emit_cleanup_function(&ctx);
    emit_main_function(&ctx);
    
    /* Cleanup */
    for (int i = 0; i < ctx.layer_count; i++) {
        free(ctx.layers[i].var_name);
    }
    free(ctx.layers);
}
