#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "../ast/ast.h"
#include "symbol_table.h"

typedef struct TypeChecker {
    Scope* current_scope;
    int error_count;
    int warning_count;
    FILE* error_stream;
} TypeChecker;

// Tensor shape operations
TensorType* tensor_type_create(int rank, const int* dims);
void tensor_type_free(TensorType* type);
TensorType* tensor_type_clone(const TensorType* type);

// Shape inference for layers
TensorType* infer_conv2d_shape(TypeChecker* tc, ASTNode* conv2d, 
                               const TensorType* input);
TensorType* infer_dense_shape(TypeChecker* tc, ASTNode* dense, 
                              const TensorType* input);
TensorType* infer_pool_shape(TypeChecker* tc, ASTNode* pool, 
                             const TensorType* input);
TensorType* infer_flatten_shape(const TensorType* input);
TensorType* infer_add_shape(TypeChecker* tc, ASTList* inputs);
TensorType* infer_concat_shape(TypeChecker* tc, ASTList* inputs);

// Validation
int validate_layer_input(TypeChecker* tc, ASTNode* layer, 
                         const char* source_name, int line);
int check_tensor_compatible(const TensorType* t1, const TensorType* t2,
                           const char* operation, int line);

#endif
