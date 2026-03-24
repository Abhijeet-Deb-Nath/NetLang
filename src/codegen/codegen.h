/*
 * NetLang Code Generator - High-Performance C Code Generation
 * 
 * Generates optimized C code for neural network inference.
 * Key optimizations:
 * - Shape specialization with compile-time constants
 * - Aligned memory allocation for AVX2
 * - Operator fusion (Conv2D+ReLU, Dense+ReLU)
 * - Direct kernel calls with inline parameters
 * 
 * Author: Abhijeet Deb Nath
 * Target: x86-64 with AVX2, GCC -O3 -march=haswell -mavx2 -mfma
 */

#ifndef NETLANG_CODEGEN_H
#define NETLANG_CODEGEN_H

#include "../ast/ast.h"
#include "../graph/graph.h"
#include "../planner/memory_plan.h"
#include "../planner/layout_plan.h"
#include "../planner/weight_pack_plan.h"
#include <stdio.h>

/* ========== CODE GENERATION CONTEXT ========== */

typedef struct LayerInfo {
    char* var_name;         /* Variable name (x, b1, etc.) */
    NodeType layer_type;    /* Layer type */
    ASTNode* layer_node;    /* AST node */
    GraphNode* graph_node;  /* Graph node */
    TensorType* input_shape;  /* Input tensor shape */
    TensorType* output_shape; /* Output tensor shape */
    int layer_index;        /* Index in .nwf file (for Conv2D/Dense) */
    int can_fuse_relu;      /* 1 if next operation is ReLU */
} LayerInfo;

typedef struct CodegenContext {
    FILE* output;           /* Output file stream */
    NetGraph* graph;        /* Graph IR */
    LayoutPlan* layout_plan;/* Layout specialization analysis */
    WeightPackPlan* weight_plan; /* Weight packing analysis */
    struct ConvExecutionPlan* conv_plan; /* Conv2D execution planning */
    struct KernelPlan* kernel_plan; /* Backend kernel selection */
    MemoryPlan* plan;       /* Activation memory plan */
    LayerInfo* layers;      /* Array of layer information */
    int layer_count;        /* Number of layers */
    int weight_layer_count; /* Number of weight-bearing layers */
    char* network_name;     /* Network name */
    char* weight_path;      /* Path to .nwf file */
    TensorType* input_shape; /* Network input shape */
} CodegenContext;

/* ========== PUBLIC API ========== */

/* Main entry point: generate C code for a NetLang network */
int generate_network_code(ASTNode* network, Scope* global_scope, FILE* output);

/* ========== INTERNAL FUNCTIONS ========== */

/* Extract layer information from AST and symbol table */
void extract_layers(CodegenContext* ctx, ASTNode* network);

/* Detect fusion opportunities (Conv2D+ReLU) */
void detect_fusion_opportunities(CodegenContext* ctx);

/* Detect cache blocking opportunities for Conv2D layers */
void detect_blocking_opportunities(CodegenContext* ctx);

/* Code generation phases */
void emit_includes(CodegenContext* ctx);
void emit_network_struct(CodegenContext* ctx);
void emit_metadata_functions(CodegenContext* ctx);
void emit_init_function(CodegenContext* ctx);
void emit_infer_function(CodegenContext* ctx);
void emit_cleanup_function(CodegenContext* ctx);
void emit_main_function(CodegenContext* ctx);

/* Layer code emission */
void emit_layer_code(CodegenContext* ctx, LayerInfo* layer, int layer_idx);
void emit_conv2d(CodegenContext* ctx, LayerInfo* layer, int layer_idx);
void emit_dense(CodegenContext* ctx, LayerInfo* layer, int layer_idx);
void emit_pooling(CodegenContext* ctx, LayerInfo* layer, int layer_idx);
void emit_flatten(CodegenContext* ctx, LayerInfo* layer, int layer_idx);
void emit_add(CodegenContext* ctx, LayerInfo* layer, int layer_idx);
void emit_concat(CodegenContext* ctx, LayerInfo* layer, int layer_idx);
void emit_activation(CodegenContext* ctx, LayerInfo* layer, ActivationType act);

/* Helper functions */
void emit_tensor_allocation(FILE* out, const char* name, TensorType* shape);
void emit_comment_separator(FILE* out, const char* title);

#endif
