/*
 * NetLang Code Generator Implementation
 * 
 * Generates high-performance C code for CNN inference.
 */

#include "codegen.h"
#include "conv_execution_plan.h"
#include "fusion_optimizer.h"
#include "blocking_config.h"
#include "kernel_selection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

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

static size_t tensor_element_count(const TensorType* shape) {
    size_t size = 1;

    if (!shape) {
        return 0;
    }

    for (int i = 0; i < shape->rank; i++) {
        size *= (size_t)shape->dims[i];
    }

    return size;
}

static int calculate_tensor_size(TensorType* shape) {
    if (!shape) return 0;
    int size = 1;
    for (int i = 0; i < shape->rank; i++) {
        size *= shape->dims[i];
    }
    return size;
}

static const ValueAllocation* get_value_allocation(CodegenContext* ctx, const GraphValue* value) {
    return memory_plan_get_allocation(ctx->plan, value);
}

static const ValueAllocation* get_storage_allocation(CodegenContext* ctx, const GraphValue* value) {
    return memory_plan_get_storage_allocation(ctx->plan, value);
}

static const DenseLayoutTransform* get_dense_layout_transform(CodegenContext* ctx, LayerInfo* layer) {
    if (!ctx || !ctx->layout_plan || !layer || !layer->graph_node) {
        return NULL;
    }

    return layout_plan_get_dense_transform(ctx->layout_plan, layer->graph_node);
}

static const PackedWeightDesc* get_packed_weight_desc(CodegenContext* ctx, LayerInfo* layer) {
    if (!ctx || !ctx->weight_plan || !layer || !layer->graph_node) {
        return NULL;
    }

    return weight_pack_plan_get_desc(ctx->weight_plan, layer->graph_node);
}

static int has_packed_weights(CodegenContext* ctx) {
    return ctx && ctx->weight_plan && ctx->weight_plan->packed_node_count > 0;
}

static const KernelChoice* get_kernel_choice(CodegenContext* ctx, int layer_idx) {
    if (!ctx || !ctx->kernel_plan) {
        return NULL;
    }

    return kernel_plan_get_choice(ctx->kernel_plan, layer_idx);
}

static const ConvExecutionChoice* get_conv_execution_choice(CodegenContext* ctx, int layer_idx) {
    if (!ctx || !ctx->conv_plan) {
        return NULL;
    }

    return conv_execution_plan_get_choice(ctx->conv_plan, layer_idx);
}

static void emit_activation_buffer(FILE* out, const char* buffer_name, const char* size_expr, ActivationType act) {
    switch (act) {
        case ACT_RELU:
            emit(out, "    relu_inplace_avx2(%s, %s);\n", buffer_name, size_expr);
            break;
        case ACT_SIGMOID:
            emit(out, "    sigmoid_inplace_avx2(%s, %s);\n", buffer_name, size_expr);
            break;
        case ACT_TANH:
            emit(out, "    tanh_inplace_avx2(%s, %s);\n", buffer_name, size_expr);
            break;
        case ACT_SOFTMAX:
            emit(out, "    softmax_inplace(%s, %s);\n", buffer_name, size_expr);
            break;
        default:
            break;
    }
}

static int get_output_slot_id(CodegenContext* ctx, LayerInfo* layer) {
    const ValueAllocation* allocation = get_value_allocation(ctx, layer->graph_node->output);
    return allocation ? allocation->slot_id : -1;
}

static void format_output_reference(CodegenContext* ctx,
                                    char* buffer,
                                    size_t buffer_size,
                                    LayerInfo* layer) {
    snprintf(buffer, buffer_size, "net->slot_%d", get_output_slot_id(ctx, layer));
}

static void format_value_reference(CodegenContext* ctx,
                                   char* buffer,
                                   size_t buffer_size,
                                   const GraphValue* value) {
    const ValueAllocation* allocation = get_storage_allocation(ctx, value);

    if (!value || !allocation || allocation->storage_kind == VALUE_STORAGE_EXTERNAL || allocation->slot_id < 0) {
        snprintf(buffer, buffer_size, "input");
        return;
    }

    snprintf(buffer, buffer_size, "net->slot_%d", allocation->slot_id);
}

static void format_dense_weight_reference(CodegenContext* ctx,
                                          char* buffer,
                                          size_t buffer_size,
                                          LayerInfo* layer,
                                          int layer_idx) {
    const PackedWeightDesc* desc = get_packed_weight_desc(ctx, layer);

    if (desc && desc->kind == PACKED_WEIGHT_DENSE_HWC) {
        snprintf(buffer, buffer_size, "net->packed_dense_weights_%d", layer_idx);
        return;
    }

    snprintf(buffer, buffer_size, "dense_weights_%d", layer_idx);
}

static void format_conv_weight_reference(CodegenContext* ctx,
                                         char* buffer,
                                         size_t buffer_size,
                                         LayerInfo* layer,
                                         int layer_idx) {
    const PackedWeightDesc* desc = get_packed_weight_desc(ctx, layer);

    if (desc && desc->kind == PACKED_WEIGHT_CONV_OC8) {
        snprintf(buffer, buffer_size, "net->packed_conv_weights_%d", layer_idx);
        return;
    }

    snprintf(buffer, buffer_size, "conv_weights_%d", layer_idx);
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
    (void)network;

    if (!ctx || !ctx->graph) return;

    ctx->layer_count = ctx->graph->topo_count;
    ctx->layers = calloc(ctx->layer_count, sizeof(LayerInfo));
    ctx->weight_layer_count = 0;

    for (int idx = 0; idx < ctx->layer_count; idx++) {
        GraphNode* graph_node = ctx->graph->topo_nodes[idx];
        LayerInfo* layer = &ctx->layers[idx];

        layer->graph_node = graph_node;
        layer->var_name = strdup(graph_node->output->storage_name);
        layer->layer_type = graph_node->op_type;
        layer->layer_node = graph_node->ast_node;
        layer->input_shape = graph_node->input_count > 0 ? graph_node->inputs[0]->type : NULL;
        layer->output_shape = graph_node->output->type;

        if (layer->layer_type == NODE_CONV2D || layer->layer_type == NODE_DENSE) {
            layer->layer_index = ctx->weight_layer_count++;
        } else {
            layer->layer_index = -1;
        }
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

/* ========== CACHE BLOCKING DETECTION ========== */

void detect_blocking_opportunities(CodegenContext* ctx) {
    int blocking_count = 0;
    
    for (int i = 0; i < ctx->layer_count; i++) {
        LayerInfo* layer = &ctx->layers[i];
        
        /* Only apply blocking to Conv2D layers */
        if (layer->layer_type != NODE_CONV2D) {
            continue;
        }
        
        ASTNode* node = layer->layer_node;
        TensorType* in_shape = layer->input_shape;
        TensorType* out_shape = layer->output_shape;
        
        if (!in_shape || !out_shape) {
            continue;
        }
        
        /* Extract dimensions */
        int out_h = out_shape->dims[0];
        int out_w = out_shape->dims[1];
        int c_in = in_shape->dims[2];
        int c_out = node->data.conv2d.filters;
        int k_h = node->data.conv2d.kernel[0];
        int k_w = node->data.conv2d.kernel[1];
        
        /* Compute blocking strategy */
        node->blocking_info = compute_blocking_strategy(
            out_h, out_w, c_in, c_out, k_h, k_w);
        
        if (node->blocking_info->l1_tile_size > 0) {
            blocking_count++;
        }
    }
    
    fprintf(stderr, "      ✓ Applied L1 cache blocking to %d Conv2D layer(s)\n", blocking_count);
}

/* ========== CODE EMISSION: HEADERS AND STRUCTURE ========== */

void emit_includes(CodegenContext* ctx) {
    emit(ctx->output, "/*\n");
    emit(ctx->output, " * Generated by NetLang Compiler\n");
    emit(ctx->output, " * Network: %s\n", ctx->network_name);
    emit(ctx->output, " * Optimizations: Shape specialization, Aligned memory, Operator fusion\n");
    emit(ctx->output, " * Compile: gcc -O3 -march=haswell -mavx2 -mfma -I. -o network network.c src/codegen/runtime.c src/codegen/kernels.c\n");
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
    emit(ctx->output, "    NetLangThreadPool* thread_pool;\n");
    emit(ctx->output, "    int thread_count;\n");
    emit(ctx->output, "    float* arena;         /* %zu bytes total */\n", ctx->plan->arena_bytes);

    for (int i = 0; i < ctx->plan->slot_count; i++) {
        MemorySlot* slot = &ctx->plan->slots[i];
        emit(ctx->output, "    float* slot_%d;       /* %zu floats, offset %zu */\n",
             slot->id, slot->element_count, slot->offset_elements);
    }

    if (has_packed_weights(ctx)) {
        emit(ctx->output, "\n");
        for (int i = 0; i < ctx->layer_count; i++) {
            const PackedWeightDesc* desc = get_packed_weight_desc(ctx, &ctx->layers[i]);
            if (!desc || desc->kind == PACKED_WEIGHT_NONE) {
                continue;
            }

            if (desc->kind == PACKED_WEIGHT_CONV_OC8) {
                emit(ctx->output, "    float* packed_conv_weights_%d; /* OC8-packed conv weights */\n", i);
            } else if (desc->kind == PACKED_WEIGHT_DENSE_HWC) {
                emit(ctx->output, "    float* packed_dense_weights_%d; /* HWC-specialized dense weights */\n", i);
            }
        }
    }
    
    emit(ctx->output, "} NetworkState;\n\n");
}

void emit_metadata_functions(CodegenContext* ctx) {
    emit(ctx->output, "/* Planner metadata for external harnesses */\n");
    emit(ctx->output, "const char* network_name(void) {\n");
    emit(ctx->output, "    return \"%s\";\n", ctx->network_name);
    emit(ctx->output, "}\n\n");

    emit(ctx->output, "size_t network_input_element_count(void) {\n");
    emit(ctx->output, "    return %d;\n", calculate_tensor_size(ctx->input_shape));
    emit(ctx->output, "}\n\n");

    emit(ctx->output, "size_t network_output_element_count(void) {\n");
    emit(ctx->output, "    return %d;\n", calculate_tensor_size(ctx->graph->output_value->type));
    emit(ctx->output, "}\n\n");

    emit(ctx->output, "size_t network_activation_arena_bytes(void) {\n");
    emit(ctx->output, "    return %zu;\n", ctx->plan->arena_bytes);
    emit(ctx->output, "}\n\n");

    emit(ctx->output, "int network_activation_slot_count(void) {\n");
    emit(ctx->output, "    return %d;\n", ctx->plan->slot_count);
    emit(ctx->output, "}\n\n");

    emit(ctx->output, "size_t network_repacked_weight_bytes(void) {\n");
    emit(ctx->output, "    return %zu;\n", ctx->weight_plan ? ctx->weight_plan->total_packed_bytes : 0);
    emit(ctx->output, "}\n\n");

    emit(ctx->output, "int network_default_thread_count(void) {\n");
    emit(ctx->output, "    return netlang_default_thread_count();\n");
    emit(ctx->output, "}\n\n");

    emit(ctx->output, "int network_thread_count(const NetworkState* net) {\n");
    emit(ctx->output, "    return net ? net->thread_count : netlang_default_thread_count();\n");
    emit(ctx->output, "}\n\n");
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
    if (ctx->weight_layer_count > 0) {
        emit(ctx->output, "    net->weights = load_weights(\"%s\");\n", ctx->weight_path);
        emit(ctx->output, "    if (!net->weights) {\n");
        emit(ctx->output, "        fprintf(stderr, \"Failed to load weights from %s\\n\");\n", ctx->weight_path);
        emit(ctx->output, "        goto fail;\n");
        emit(ctx->output, "    }\n\n");
    } else {
        emit(ctx->output, "    net->weights = NULL;\n\n");
    }

    emit_comment_separator(ctx->output, "Initialize Execution Runtime");
    emit(ctx->output, "    net->thread_pool = netlang_thread_pool_create(0);\n");
    emit(ctx->output, "    if (!net->thread_pool) {\n");
    emit(ctx->output, "        fprintf(stderr, \"Failed to create thread pool\\n\");\n");
    emit(ctx->output, "        goto fail;\n");
    emit(ctx->output, "    }\n");
    emit(ctx->output, "    net->thread_count = netlang_thread_pool_thread_count(net->thread_pool);\n\n");
    
    /* Allocate activation arena */
    emit_comment_separator(ctx->output, "Allocate Activation Arena");
    if (ctx->plan->arena_bytes > 0) {
        emit(ctx->output, "    net->arena = aligned_alloc_64(%zu);\n", ctx->plan->arena_bytes);
        emit(ctx->output, "    if (!net->arena) {\n");
        emit(ctx->output, "        fprintf(stderr, \"Failed to allocate activation arena\\n\");\n");
        emit(ctx->output, "        goto fail;\n");
        emit(ctx->output, "    }\n");
        for (int i = 0; i < ctx->plan->slot_count; i++) {
            MemorySlot* slot = &ctx->plan->slots[i];
            emit(ctx->output, "    net->slot_%d = net->arena + %zu;\n", slot->id, slot->offset_elements);
        }
    } else {
        emit(ctx->output, "    net->arena = NULL;\n");
    }

    if (has_packed_weights(ctx)) {
        emit(ctx->output, "\n");
        emit_comment_separator(ctx->output, "Packed Weights");
        for (int i = 0; i < ctx->layer_count; i++) {
            const PackedWeightDesc* desc = get_packed_weight_desc(ctx, &ctx->layers[i]);
            if (!desc || desc->kind == PACKED_WEIGHT_NONE) {
                continue;
            }

            if (desc->kind == PACKED_WEIGHT_CONV_OC8) {
                emit(ctx->output, "    net->packed_conv_weights_%d = repack_conv2d_weights_oc8(\n", i);
                emit(ctx->output, "        get_layer_weights(net->weights, %d),\n", ctx->layers[i].layer_index);
                emit(ctx->output, "        %d, %d, %d, %d\n",
                     desc->out_channels, desc->in_channels, desc->kernel_h, desc->kernel_w);
                emit(ctx->output, "    );\n");
                emit(ctx->output, "    if (!net->packed_conv_weights_%d) {\n", i);
            } else {
                emit(ctx->output, "    net->packed_dense_weights_%d = repack_dense_weights_chw_to_hwc(\n", i);
                emit(ctx->output, "        get_layer_weights(net->weights, %d),\n", ctx->layers[i].layer_index);
                emit(ctx->output, "        %d,\n", ctx->layers[i].layer_node->data.dense.units);
                emit(ctx->output, "        %d, %d, %d\n", desc->height, desc->width, desc->channels);
                emit(ctx->output, "    );\n");
                emit(ctx->output, "    if (!net->packed_dense_weights_%d) {\n", i);
            }
            emit(ctx->output, "        fprintf(stderr, \"Failed to prepare packed weights for layer %d\\n\");\n", i);
            emit(ctx->output, "        goto fail;\n");
            emit(ctx->output, "    }\n");
        }
    }

    emit(ctx->output, "\n    return net;\n");
    emit(ctx->output, "\nfail:\n");
    if (has_packed_weights(ctx)) {
        for (int i = 0; i < ctx->layer_count; i++) {
            const PackedWeightDesc* desc = get_packed_weight_desc(ctx, &ctx->layers[i]);
            if (!desc || desc->kind == PACKED_WEIGHT_NONE) {
                continue;
            }
            if (desc->kind == PACKED_WEIGHT_CONV_OC8) {
                emit(ctx->output, "    aligned_free(net ? net->packed_conv_weights_%d : NULL);\n", i);
            } else {
                emit(ctx->output, "    aligned_free(net ? net->packed_dense_weights_%d : NULL);\n", i);
            }
        }
    }
    emit(ctx->output, "    aligned_free(net ? net->arena : NULL);\n");
    emit(ctx->output, "    if (net && net->thread_pool) netlang_thread_pool_destroy(net->thread_pool);\n");
    emit(ctx->output, "    if (net && net->weights) unload_weights(net->weights);\n");
    emit(ctx->output, "    free(net);\n");
    emit(ctx->output, "    return NULL;\n");
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
    GraphValue* output_value = ctx->graph->output_value;
    int output_size = calculate_tensor_size(output_value->type);
    emit(ctx->output, "    /* Copy output */\n");
    if (output_value->producer) {
        const ValueAllocation* allocation = get_storage_allocation(ctx, output_value);
        if (allocation) {
            if (allocation->storage_kind == VALUE_STORAGE_EXTERNAL || allocation->slot_id < 0) {
                emit(ctx->output, "    memcpy(output, input, %d * sizeof(float));\n", output_size);
            } else {
                emit(ctx->output, "    memcpy(output, net->slot_%d, %d * sizeof(float));\n",
                     allocation->slot_id, output_size);
            }
        } else {
            emit(ctx->output, "    /* ERROR: missing allocation for final output */\n");
        }
    } else {
        emit(ctx->output, "    memcpy(output, input, %d * sizeof(float));\n", output_size);
    }
    
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
        case NODE_ADD:
            emit_add(ctx, layer, layer_idx);
            break;
        case NODE_CONCAT:
            emit_concat(ctx, layer, layer_idx);
            break;
        default:
            emit(ctx->output, "    /* TODO: Layer type %d */\n", layer->layer_type);
    }
}

void emit_conv2d(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    ASTNode* node = layer->layer_node;
    const PackedWeightDesc* packed_desc = get_packed_weight_desc(ctx, layer);
    const ConvExecutionChoice* exec_choice = get_conv_execution_choice(ctx, layer_idx);
    const KernelChoice* kernel_choice = get_kernel_choice(ctx, layer_idx);
    int filters = node->data.conv2d.filters;
    int kernel_h = node->data.conv2d.kernel[0];
    int kernel_w = node->data.conv2d.kernel[1];
    int stride = node->data.conv2d.stride;
    int padding = node->data.conv2d.padding;
    ActivationType act = node->data.conv2d.activation;
    char weight_name[64];
    
    /* Get input tensor reference from graph */
    char input_name[64];
    char output_name[64];
    format_value_reference(ctx, input_name, sizeof(input_name), layer->graph_node->inputs[0]);
    format_output_reference(ctx, output_name, sizeof(output_name), layer);
    
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
    int needs_tile_size = 0;
    int fused_activation = 0;
    int spatial_block_width = exec_choice ? exec_choice->spatial_block_width : 1;

    /* Get weights and bias from .nwf file */
    if (!(packed_desc && packed_desc->kind == PACKED_WEIGHT_CONV_OC8)) {
        emit(ctx->output, "    const float* conv_weights_%d = get_layer_weights(net->weights, %d);\n", 
             layer_idx, layer->layer_index);
    }
    emit(ctx->output, "    const float* conv_bias_%d = get_layer_bias(net->weights, %d);\n", 
         layer_idx, layer->layer_index);
    format_conv_weight_reference(ctx, weight_name, sizeof(weight_name), layer, layer_idx);
    
    /* Emit optimized Conv2D call based on available optimizations */
    emit(ctx->output, "    /* Conv2D: [%d,%d,%d] -> [%d,%d,%d], K=%dx%d, S=%d, P=%d",
         in_h, in_w, in_c, out_h, out_w, filters, kernel_h, kernel_w, stride, padding);
    if (spatial_block_width > 1) {
        emit(ctx->output, ", OWx%d", spatial_block_width);
    }

    switch (kernel_choice ? kernel_choice->conv_kind : CONV_KERNEL_LEGACY) {
        case CONV_KERNEL_PACKED_OC8_FUSED_BLOCKED:
            emit(ctx->output, " [PACKED+FUSED+BLOCKED] */\n");
            emit(ctx->output, "    conv2d_relu_packed_oc8_blocked_avx2(\n");
            needs_tile_size = 1;
            fused_activation = 1;
            break;
        case CONV_KERNEL_PACKED_OC8_FUSED:
            emit(ctx->output, " [PACKED+FUSED] */\n");
            emit(ctx->output, "    conv2d_relu_packed_oc8_forward_avx2(\n");
            fused_activation = 1;
            break;
        case CONV_KERNEL_PACKED_OC8_BLOCKED:
            emit(ctx->output, " [PACKED+BLOCKED] */\n");
            emit(ctx->output, "    conv2d_packed_oc8_blocked_avx2(\n");
            needs_tile_size = 1;
            break;
        case CONV_KERNEL_PACKED_OC8:
            emit(ctx->output, " [PACKED] */\n");
            emit(ctx->output, "    conv2d_packed_oc8_forward_avx2(\n");
            break;
        case CONV_KERNEL_FUSED_BLOCKED:
            emit(ctx->output, " [FUSED+BLOCKED] */\n");
            emit(ctx->output, "    conv2d_relu_blocked_avx2(\n");
            needs_tile_size = 1;
            fused_activation = 1;
            break;
        case CONV_KERNEL_FUSED:
            emit(ctx->output, " [FUSED] */\n");
            emit(ctx->output, "    conv2d_relu_forward_avx2(\n");
            fused_activation = 1;
            break;
        case CONV_KERNEL_BLOCKED:
            emit(ctx->output, " [BLOCKED] */\n");
            emit(ctx->output, "    conv2d_blocked_avx2(\n");
            needs_tile_size = 1;
            break;
        case CONV_KERNEL_LEGACY:
        default:
            emit(ctx->output, " */\n");
            emit(ctx->output, "    conv2d_forward_avx2(\n");
            break;
    }
    
    emit(ctx->output, "        %s, %s, conv_bias_%d, %s,\n",
         input_name, weight_name, layer_idx, output_name);
    emit(ctx->output, "        %d, %d, %d,  /* in: H, W, C */\n", in_h, in_w, in_c);
    emit(ctx->output, "        %d, %d,      /* kernel: H, W */\n", kernel_h, kernel_w);
    emit(ctx->output, "        %d,          /* out channels */\n", filters);
    emit(ctx->output, "        %d, %d",     stride, padding);
    
    /* Add tile size parameter if using blocked kernel */
    if (needs_tile_size) {
        emit(ctx->output, ",      /* stride, padding */\n");
        if (kernel_choice &&
            (kernel_choice->conv_kind == CONV_KERNEL_PACKED_OC8_BLOCKED ||
             kernel_choice->conv_kind == CONV_KERNEL_PACKED_OC8_FUSED_BLOCKED)) {
            emit(ctx->output, "        %d,          /* tile size */\n", 
                 node->blocking_info->l1_tile_size);
            emit(ctx->output, "        %d,          /* output-width micro-tile */\n", spatial_block_width);
            emit(ctx->output, "        net->thread_pool\n");
        } else {
            emit(ctx->output, "        %d           /* tile size */\n", 
                 node->blocking_info->l1_tile_size);
        }
    } else {
        emit(ctx->output, "       /* stride, padding */");
        if (kernel_choice &&
            (kernel_choice->conv_kind == CONV_KERNEL_PACKED_OC8 ||
             kernel_choice->conv_kind == CONV_KERNEL_PACKED_OC8_FUSED)) {
            emit(ctx->output, ",\n");
            emit(ctx->output, "        %d,          /* output-width micro-tile */\n", spatial_block_width);
            emit(ctx->output, "        net->thread_pool\n");
        } else {
            emit(ctx->output, "\n");
        }
    }
    
    emit(ctx->output, "    );\n");
    
    /* Apply activation if not fused */
    if (!fused_activation && act != ACT_NONE && act != ACT_LINEAR) {
        emit_activation(ctx, layer, act);
    }
}

void emit_dense(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    ASTNode* node = layer->layer_node;
    const DenseLayoutTransform* transform = get_dense_layout_transform(ctx, layer);
    const KernelChoice* kernel_choice = get_kernel_choice(ctx, layer_idx);
    char dense_weight_name[64];
    
    if (!node) {
        emit(ctx->output, "    /* ERROR: NULL layer node */\n");
        return;
    }
    
    int units = node->data.dense.units;
    ActivationType act = node->data.dense.activation;
    
    /* Get input tensor reference from graph */
    char input_name[64];
    char output_name[64];
    format_value_reference(ctx, input_name, sizeof(input_name), layer->graph_node->inputs[0]);
    format_output_reference(ctx, output_name, sizeof(output_name), layer);
    
    /* Get shapes */
    TensorType* in_shape = layer->input_shape;
    
    if (!in_shape) {
        emit(ctx->output, "    /* ERROR: Missing input shape for Dense */\n");
        return;
    }
    
    int in_features = calculate_tensor_size(in_shape);
    
    /* Get weights and bias */
    if (!(transform && transform->enabled)) {
        emit(ctx->output, "    const float* dense_weights_%d = get_layer_weights(net->weights, %d);\n", 
             layer_idx, layer->layer_index);
    }
    emit(ctx->output, "    const float* dense_bias_%d = get_layer_bias(net->weights, %d);\n", 
         layer_idx, layer->layer_index);
    format_dense_weight_reference(ctx, dense_weight_name, sizeof(dense_weight_name), layer, layer_idx);
    
    emit(ctx->output, "    /* Dense: %d -> %d", in_features, units);
    if (transform && transform->enabled) {
        emit(ctx->output, " [layout-specialized HWC path]");
    }
    emit(ctx->output, " */\n");

    switch (kernel_choice ? kernel_choice->dense_kind : DENSE_KERNEL_BASE) {
        case DENSE_KERNEL_PACKED_HWC_FUSED:
        case DENSE_KERNEL_FUSED:
            emit(ctx->output, "    dense_relu_forward_avx2(\n");
            break;
        case DENSE_KERNEL_PACKED_HWC:
        case DENSE_KERNEL_BASE:
        default:
            emit(ctx->output, "    dense_forward_avx2(\n");
            break;
    }
    
    emit(ctx->output, "        %s, %s, dense_bias_%d, %s,\n",
         input_name, dense_weight_name, layer_idx, output_name);
    emit(ctx->output, "        %d, %d  /* in_features, out_features */\n", in_features, units);
    emit(ctx->output, "    );\n");
    
    /* Apply activation if not fused */
    if ((!kernel_choice || (kernel_choice->dense_kind != DENSE_KERNEL_FUSED &&
                            kernel_choice->dense_kind != DENSE_KERNEL_PACKED_HWC_FUSED)) &&
        act != ACT_NONE && act != ACT_LINEAR) {
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
    char output_name[64];
    format_value_reference(ctx, input_name, sizeof(input_name), layer->graph_node->inputs[0]);
    format_output_reference(ctx, output_name, sizeof(output_name), layer);
    
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
    emit(ctx->output, "        %s, %s,\n", input_name, output_name);
    emit(ctx->output, "        %d, %d, %d,  /* in: H, W, C */\n", in_h, in_w, channels);
    emit(ctx->output, "        %d, %d,      /* pool: H, W */\n", pool_h, pool_w);
    emit(ctx->output, "        %d, %d       /* stride, padding */\n", stride, padding);
    emit(ctx->output, "    );\n");
}

void emit_flatten(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    char input_name[64];
    char output_name[64];
    const ValueAllocation* allocation = get_value_allocation(ctx, layer->graph_node->output);
    (void)layer_idx;
    format_value_reference(ctx, input_name, sizeof(input_name), layer->graph_node->inputs[0]);
    format_output_reference(ctx, output_name, sizeof(output_name), layer);
    
    if (!layer->input_shape) {
        emit(ctx->output, "    /* ERROR: Missing input shape for flatten */\n");
        return;
    }
    
    int H = layer->input_shape->dims[0];
    int W = layer->input_shape->dims[1];
    int C = layer->input_shape->dims[2];
    int size = H * W * C;

    if (allocation && allocation->storage_kind == VALUE_STORAGE_ALIAS) {
        emit(ctx->output, "    /* Flatten elided: Dense consumer(s) read HWC storage directly via repacked weights [%d] */\n",
             size);
        return;
    }
    
    /* Convert HWC to CHW order for ONNX/PyTorch compatibility */
    emit(ctx->output, "    /* Flatten: HWC [%d,%d,%d] -> CHW order [%d] for dense layer compatibility */\n",
         H, W, C, size);
    emit(ctx->output, "    flatten_hwc_to_chw(%s, %s, %d, %d, %d);\n",
         input_name, output_name, H, W, C);
}

void emit_add(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    GraphNode* node = layer->graph_node;
    TensorType* out_shape = layer->output_shape;
    char output_name[64];

    if (!node || node->input_count == 0 || !out_shape) {
        emit(ctx->output, "    /* ERROR: Invalid graph information for Add */\n");
        return;
    }

    format_output_reference(ctx, output_name, sizeof(output_name), layer);

    emit(ctx->output, "    /* Add: %d inputs -> [%d] flattened elements */\n",
         node->input_count, calculate_tensor_size(out_shape));
    emit(ctx->output, "    const float* add_inputs_%d[%d] = {\n", layer_idx, node->input_count);
    for (int i = 0; i < node->input_count; i++) {
        char input_name[64];
        format_value_reference(ctx, input_name, sizeof(input_name), node->inputs[i]);
        emit(ctx->output, "        %s%s\n", input_name, (i + 1 < node->input_count) ? "," : "");
    }
    emit(ctx->output, "    };\n");

    emit(ctx->output, "    add_forward(\n");
    emit(ctx->output, "        add_inputs_%d,\n", layer_idx);
    emit(ctx->output, "        %s,\n", output_name);
    emit(ctx->output, "        %d,\n", node->input_count);
    emit(ctx->output, "        %d\n", calculate_tensor_size(out_shape));
    emit(ctx->output, "    );\n");
}

void emit_concat(CodegenContext* ctx, LayerInfo* layer, int layer_idx) {
    GraphNode* node = layer->graph_node;
    TensorType* out_shape = layer->output_shape;
    char output_name[64];

    if (!node || node->input_count == 0 || !out_shape) {
        emit(ctx->output, "    /* ERROR: Invalid graph information for Concat */\n");
        return;
    }

    format_output_reference(ctx, output_name, sizeof(output_name), layer);

    int H = node->inputs[0]->type->dims[0];
    int W = node->inputs[0]->type->dims[1];
    int total_channels = out_shape->dims[2];

    emit(ctx->output, "    /* Concat: %d inputs -> [%d,%d,%d] */\n",
         node->input_count, H, W, total_channels);
    emit(ctx->output, "    const float* concat_inputs_%d[%d] = {\n", layer_idx, node->input_count);
    for (int i = 0; i < node->input_count; i++) {
        char input_name[64];
        format_value_reference(ctx, input_name, sizeof(input_name), node->inputs[i]);
        emit(ctx->output, "        %s%s\n", input_name, (i + 1 < node->input_count) ? "," : "");
    }
    emit(ctx->output, "    };\n");

    emit(ctx->output, "    const int concat_channels_%d[%d] = { ", layer_idx, node->input_count);
    for (int i = 0; i < node->input_count; i++) {
        emit(ctx->output, "%d%s",
             node->inputs[i]->type->dims[2],
             (i + 1 < node->input_count) ? ", " : " ");
    }
    emit(ctx->output, "};\n");

    emit(ctx->output, "    concat_forward(\n");
    emit(ctx->output, "        concat_inputs_%d,\n", layer_idx);
    emit(ctx->output, "        %s,\n", output_name);
    emit(ctx->output, "        %d,\n", node->input_count);
    emit(ctx->output, "        %d, %d,\n", H, W);
    emit(ctx->output, "        concat_channels_%d,\n", layer_idx);
    emit(ctx->output, "        %d\n", total_channels);
    emit(ctx->output, "    );\n");
}

void emit_activation(CodegenContext* ctx, LayerInfo* layer, ActivationType act) {
    int size = calculate_tensor_size(layer->output_shape);
    char output_name[64];
    format_output_reference(ctx, output_name, sizeof(output_name), layer);
    
    switch (act) {
        case ACT_RELU:
            emit(ctx->output, "    relu_inplace_avx2(%s, %d);\n", output_name, size);
            break;
        case ACT_SIGMOID:
            emit(ctx->output, "    sigmoid_inplace_avx2(%s, %d);\n", output_name, size);
            break;
        case ACT_TANH:
            emit(ctx->output, "    tanh_inplace_avx2(%s, %d);\n", output_name, size);
            break;
        case ACT_SOFTMAX:
            emit(ctx->output, "    softmax_inplace(%s, %d);\n", output_name, size);
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

    if (has_packed_weights(ctx)) {
        for (int i = 0; i < ctx->layer_count; i++) {
            const PackedWeightDesc* desc = get_packed_weight_desc(ctx, &ctx->layers[i]);
            if (!desc || desc->kind == PACKED_WEIGHT_NONE) {
                continue;
            }

            if (desc->kind == PACKED_WEIGHT_CONV_OC8) {
                emit(ctx->output, "    aligned_free(net->packed_conv_weights_%d);\n", i);
            } else {
                emit(ctx->output, "    aligned_free(net->packed_dense_weights_%d);\n", i);
            }
        }
        emit(ctx->output, "\n");
    }

    emit(ctx->output, "    aligned_free(net->arena);\n");
    emit(ctx->output, "    netlang_thread_pool_destroy(net->thread_pool);\n");
    emit(ctx->output, "    unload_weights(net->weights);\n");
    emit(ctx->output, "    free(net);\n");
    emit(ctx->output, "}\n\n");
}

/* ========== MAIN FUNCTION ========== */

void emit_main_function(CodegenContext* ctx) {
    emit(ctx->output, "#ifndef NETLANG_NO_MAIN\n");
    emit(ctx->output, "/* Main: simple test harness */\n");
    emit(ctx->output, "int main() {\n");
    emit(ctx->output, "    printf(\"Initializing network: %s\\n\");\n", ctx->network_name);
    emit(ctx->output, "    NetworkState* net = network_init();\n");
    emit(ctx->output, "    if (!net) return 1;\n\n");
    
    /* Allocate input and output */
    int input_size = calculate_tensor_size(ctx->input_shape);
    int output_size = calculate_tensor_size(ctx->graph->output_value->type);
    
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
    emit(ctx->output, "#endif /* NETLANG_NO_MAIN */\n");
}

/* ========== MAIN ENTRY POINT ========== */

void generate_network_code(ASTNode* network, Scope* global_scope, FILE* output) {
    (void)global_scope;

    if (!network || network->type != NODE_NETWORK) {
        fprintf(stderr, "ERROR: Invalid network node\n");
        return;
    }

    /* Initialize context */
    CodegenContext ctx = {0};
    ctx.output = output;
    ctx.network_name = network->data.network.name;

    ctx.graph = netgraph_build(network, stderr);
    if (!ctx.graph) {
        fprintf(stderr, "ERROR: Graph lowering failed\n");
        return;
    }

    ctx.layout_plan = layout_plan_build(ctx.graph);
    if (!ctx.layout_plan) {
        fprintf(stderr, "ERROR: Layout analysis failed\n");
        netgraph_free(ctx.graph);
        return;
    }

    ctx.weight_plan = weight_pack_plan_build(ctx.graph, ctx.layout_plan);
    if (!ctx.weight_plan) {
        fprintf(stderr, "ERROR: Weight packing analysis failed\n");
        layout_plan_free(ctx.layout_plan);
        netgraph_free(ctx.graph);
        return;
    }

    ctx.weight_path = ctx.graph->weight_path;
    ctx.input_shape = ctx.graph->input_type;
    ctx.plan = memory_plan_build(ctx.graph, ctx.layout_plan);
    if (!ctx.plan) {
        fprintf(stderr, "ERROR: Memory planning failed\n");
        weight_pack_plan_free(ctx.weight_plan);
        layout_plan_free(ctx.layout_plan);
        netgraph_free(ctx.graph);
        return;
    }

    if (ctx.graph->node_count == 0) {
        fprintf(stderr, "ERROR: Graph contains no executable nodes\n");
        memory_plan_free(ctx.plan);
        weight_pack_plan_free(ctx.weight_plan);
        layout_plan_free(ctx.layout_plan);
        netgraph_free(ctx.graph);
        return;
    }

    for (int i = 0; i < ctx.graph->node_count; i++) {
        NodeType type = ctx.graph->nodes[i]->op_type;
        if (type == NODE_BATCHNORM || type == NODE_LAYERNORM || type == NODE_MODULE_CALL) {
            fprintf(stderr, "ERROR: Graph node type %s is not yet supported by code generation\n",
                    netgraph_node_type_name(type));
            memory_plan_free(ctx.plan);
            weight_pack_plan_free(ctx.weight_plan);
            layout_plan_free(ctx.layout_plan);
            netgraph_free(ctx.graph);
            return;
        }
    }
    
    /* Extract network structure */
    extract_layers(&ctx, network);
    detect_fusion_opportunities(&ctx);
    detect_blocking_opportunities(&ctx);
    ctx.conv_plan = conv_execution_plan_build(&ctx);
    if (!ctx.conv_plan) {
        fprintf(stderr, "ERROR: Conv execution planning failed\n");
        for (int i = 0; i < ctx.layer_count; i++) {
            free(ctx.layers[i].var_name);
        }
        free(ctx.layers);
        memory_plan_free(ctx.plan);
        weight_pack_plan_free(ctx.weight_plan);
        layout_plan_free(ctx.layout_plan);
        netgraph_free(ctx.graph);
        return;
    }
    ctx.kernel_plan = kernel_plan_build(&ctx);
    if (!ctx.kernel_plan) {
        fprintf(stderr, "ERROR: Kernel selection failed\n");
        for (int i = 0; i < ctx.layer_count; i++) {
            free(ctx.layers[i].var_name);
        }
        free(ctx.layers);
        conv_execution_plan_free(ctx.conv_plan);
        memory_plan_free(ctx.plan);
        weight_pack_plan_free(ctx.weight_plan);
        layout_plan_free(ctx.layout_plan);
        netgraph_free(ctx.graph);
        return;
    }
    
    /* Generate code */
    emit_includes(&ctx);
    emit_network_struct(&ctx);
    emit_metadata_functions(&ctx);
    emit_init_function(&ctx);
    emit_infer_function(&ctx);
    emit_cleanup_function(&ctx);
    emit_main_function(&ctx);
    
    /* Cleanup */
    for (int i = 0; i < ctx.layer_count; i++) {
        free(ctx.layers[i].var_name);
    }
    free(ctx.layers);
    conv_execution_plan_free(ctx.conv_plan);
    kernel_plan_free(ctx.kernel_plan);
    memory_plan_free(ctx.plan);
    weight_pack_plan_free(ctx.weight_plan);
    layout_plan_free(ctx.layout_plan);
    netgraph_free(ctx.graph);
}
