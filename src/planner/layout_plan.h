#ifndef NETLANG_LAYOUT_PLAN_H
#define NETLANG_LAYOUT_PLAN_H

#include "../graph/graph.h"
#include <stddef.h>

typedef struct DenseLayoutTransform {
    int enabled;
    int flatten_value_id;
    int source_value_id;
    int height;
    int width;
    int channels;
} DenseLayoutTransform;

typedef struct LayoutPlan {
    const NetGraph* graph;
    int* alias_source_ids;                 /* Indexed by value id, -1 if materialized */
    DenseLayoutTransform* dense_transforms;/* Indexed by node id */
    int alias_value_count;
    int transformed_dense_count;
    size_t repacked_weight_bytes;
} LayoutPlan;

LayoutPlan* layout_plan_build(const NetGraph* graph);
void layout_plan_free(LayoutPlan* plan);

int layout_plan_get_alias_source_id(const LayoutPlan* plan, const GraphValue* value);
const GraphValue* layout_plan_get_alias_source(const LayoutPlan* plan, const GraphValue* value);
const DenseLayoutTransform* layout_plan_get_dense_transform(const LayoutPlan* plan, const GraphNode* node);

#endif /* NETLANG_LAYOUT_PLAN_H */
