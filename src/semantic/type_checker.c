#include "type_checker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== TENSOR TYPE HELPERS ========== */

TensorType* tensor_type_create(int rank, const int* dims) {
    TensorType* type = (TensorType*)calloc(1, sizeof(TensorType));
    type->rank = rank;
    type->dtype = "float32";  // Default type for CNN
    
    if (dims) {
        for (int i = 0; i < rank && i < 4; i++) {
            type->dims[i] = dims[i];
        }
    }
    
    return type;
}

void tensor_type_free(TensorType* type) {
    if (type) {
        free(type);
    }
}

TensorType* tensor_type_clone(const TensorType* type) {
    if (!type) return NULL;
    
    TensorType* clone = tensor_type_create(type->rank, type->dims);
    clone->dtype = type->dtype;
    return clone;
}

/* ========== SHAPE INFERENCE ========== */

TensorType* infer_conv2d_shape(TypeChecker* tc, ASTNode* conv2d, 
                            const TensorType* input) {
    if (!input || input->rank != 3) {
        fprintf(tc->error_stream, "Conv2D requires 3D input [H,W,C] (got rank %d)\n",
                input ? input->rank : 0);
        tc->error_count++;
        return NULL;
    }
    
    TensorType* output = tensor_type_clone(input);
    int filters = conv2d->data.conv2d.filters;
    int kernel_h = conv2d->data.conv2d.kernel[0];
    int kernel_w = conv2d->data.conv2d.kernel[1];
    int stride = conv2d->data.conv2d.stride;
    int padding = conv2d->data.conv2d.padding;
    
    // output_height = (input_height + 2*padding - kernel_h)/stride + 1
    // output_width = (input_width + 2*padding - kernel_w)/stride + 1
    // output_channels = filters
    output->dims[0] = (input->dims[0] + 2*padding - kernel_h) / stride + 1;
    output->dims[1] = (input->dims[1] + 2*padding - kernel_w) / stride + 1;
    output->dims[2] = filters;
    
    return output;
}

TensorType* infer_dense_shape(TypeChecker* tc, ASTNode* dense, 
                              const TensorType* input) {
    if (!input) return NULL;
    
    // Dense requires 1D input [features]
    // If input is 3D (from Conv/Pool), must flatten first
    if (input->rank == 3) {
        fprintf(tc->error_stream, 
                "Warning: Dense expects 1D input [features], got 3D [H,W,C]. Insert Flatten() first.\n");
        tc->warning_count++;
    }
    
    // Output is 1D: [units]
    int dims[1] = { dense->data.dense.units };
    TensorType* output = tensor_type_create(1, dims);
    
    return output;
}

TensorType* infer_pool_shape(TypeChecker* tc, ASTNode* pool, 
                             const TensorType* input) {
    if (!input || input->rank != 3) {
        fprintf(tc->error_stream, "Pooling requires 3D input [H,W,C]\n");
        tc->error_count++;
        return NULL;
    }
    
    TensorType* output = tensor_type_clone(input);
    int pool_h = pool->data.pooling.pool[0];
    int pool_w = pool->data.pooling.pool[1];
    int stride = pool->data.pooling.stride;
    int padding = pool->data.pooling.padding;
    
    // Default stride to pool size if not specified (stride=0)
    if (stride == 0) {
        stride = pool_h;
    }
    
    // Output spatial dimensions: [H', W', C]
    // Channels unchanged
    output->dims[0] = (input->dims[0] + 2*padding - pool_h) / stride + 1;
    output->dims[1] = (input->dims[1] + 2*padding - pool_w) / stride + 1;
    output->dims[2] = input->dims[2];
    
    return output;
}

TensorType* infer_flatten_shape(const TensorType* input) {
    if (!input) return NULL;
    
    // Flatten 3D [H, W, C] -> 1D [H*W*C]
    int features = 1;
    if (input->rank == 3) {
        features = input->dims[0] * input->dims[1] * input->dims[2];
    } else if (input->rank == 1) {
        features = input->dims[0];  // Already flat
    }
    
    int dims[1] = { features };
    TensorType* output = tensor_type_create(1, dims);
    
    return output;
}

TensorType* infer_add_shape(TypeChecker* tc, ASTList* inputs) {
    if (!inputs || !inputs->head) {
        fprintf(tc->error_stream, "Add requires at least one input\n");
        tc->error_count++;
        return NULL;
    }

    ASTNode* input_expr = inputs->head;
    TensorType* base_type = NULL;

    while (input_expr) {
        if (input_expr->type != NODE_IDENTIFIER) {
            fprintf(tc->error_stream, "Add inputs must be identifiers\n");
            tc->error_count++;
            return NULL;
        }

        Symbol* sym = scope_lookup(tc->current_scope,
                                   input_expr->data.identifier.name, 1);
        if (!sym || !sym->tensor_type) {
            fprintf(tc->error_stream, "Undefined tensor '%s' in Add\n",
                    input_expr->data.identifier.name);
            tc->error_count++;
            return NULL;
        }

        if (!base_type) {
            base_type = sym->tensor_type;
        } else if (!check_tensor_compatible(base_type, sym->tensor_type, "Add", input_expr->line_number)) {
            tc->error_count++;
            return NULL;
        }

        input_expr = input_expr->next;
    }

    return tensor_type_clone(base_type);
}

TensorType* infer_concat_shape(TypeChecker* tc, ASTList* inputs) {
    if (!inputs || !inputs->head) {
        fprintf(tc->error_stream, "Concat requires at least one input\n");
        tc->error_count++;
        return NULL;
    }

    int dims[3] = {0, 0, 0};
    ASTNode* input_expr = inputs->head;
    int total_channels = 0;

    while (input_expr) {
        if (input_expr->type != NODE_IDENTIFIER) {
            fprintf(tc->error_stream, "Concat inputs must be identifiers\n");
            tc->error_count++;
            return NULL;
        }

        Symbol* sym = scope_lookup(tc->current_scope,
                                   input_expr->data.identifier.name, 1);
        if (!sym || !sym->tensor_type) {
            fprintf(tc->error_stream, "Undefined tensor '%s' in Concat\n",
                    input_expr->data.identifier.name);
            tc->error_count++;
            return NULL;
        }

        TensorType* type = sym->tensor_type;
        if (type->rank != 3) {
            fprintf(tc->error_stream, "Concat currently requires 3D inputs [H,W,C]\n");
            tc->error_count++;
            return NULL;
        }

        if (total_channels == 0) {
            dims[0] = type->dims[0];
            dims[1] = type->dims[1];
        } else if (type->dims[0] != dims[0] || type->dims[1] != dims[1]) {
            fprintf(tc->error_stream, "Concat inputs must have matching H and W dimensions\n");
            tc->error_count++;
            return NULL;
        }

        total_channels += type->dims[2];
        input_expr = input_expr->next;
    }

    dims[2] = total_channels;
    return tensor_type_create(3, dims);
}

/* ========== VALIDATION ========== */

int validate_layer_input(TypeChecker* tc, ASTNode* layer, 
                        const char* source_name, int line) {
    // Look up source symbol
    Symbol* sym = scope_lookup(tc->current_scope, source_name, 1);
    
    if (!sym) {
        fprintf(tc->error_stream, "Line %d: Undefined variable '%s'\n",
                line, source_name);
        tc->error_count++;
        return 0;
    }
    
    // Validate tensor type if available
    if (!sym->tensor_type) {
        fprintf(tc->error_stream, 
                "Line %d: Variable '%s' has no inferred type\n",
                line, source_name);
        tc->warning_count++;
        return 0;
    }
    
    return 1;
}

int check_tensor_compatible(const TensorType* t1, const TensorType* t2,
                           const char* operation, int line) {
    if (!t1 || !t2) return 0;
    
    // For most operations, all dimensions must match
    if (t1->rank != t2->rank) {
        fprintf(stderr, "Line %d: %s - rank mismatch (%d vs %d)\n",
                line, operation, t1->rank, t2->rank);
        return 0;
    }
    
    // Check all dimensions match
    for (int i = 0; i < t1->rank; i++) {
        if (t1->dims[i] != t2->dims[i]) {
            fprintf(stderr, "Line %d: %s - dimension %d mismatch (%d vs %d)\n",
                    line, operation, i, t1->dims[i], t2->dims[i]);
            return 0;
        }
    }
    
    return 1;
}
