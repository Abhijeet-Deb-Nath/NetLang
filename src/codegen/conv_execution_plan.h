#ifndef NETLANG_CONV_EXECUTION_PLAN_H
#define NETLANG_CONV_EXECUTION_PLAN_H

#include "codegen.h"

typedef struct ConvExecutionChoice {
    int spatial_block_width; /* Number of adjacent output positions to compute together */
} ConvExecutionChoice;

typedef struct ConvExecutionPlan {
    ConvExecutionChoice* layer_choices; /* Indexed by LayerInfo array index */
    int layer_count;
} ConvExecutionPlan;

ConvExecutionPlan* conv_execution_plan_build(CodegenContext* ctx);
void conv_execution_plan_free(ConvExecutionPlan* plan);
const ConvExecutionChoice* conv_execution_plan_get_choice(const ConvExecutionPlan* plan, int layer_index);

#endif /* NETLANG_CONV_EXECUTION_PLAN_H */
