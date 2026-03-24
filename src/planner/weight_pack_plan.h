#ifndef NETLANG_WEIGHT_PACK_PLAN_H
#define NETLANG_WEIGHT_PACK_PLAN_H

#include "../graph/graph.h"
#include "layout_plan.h"
#include <stddef.h>

typedef enum PackedWeightKind {
    PACKED_WEIGHT_NONE = 0,
    PACKED_WEIGHT_CONV_OC8 = 1,
    PACKED_WEIGHT_DENSE_HWC = 2
} PackedWeightKind;

typedef struct PackedWeightDesc {
    PackedWeightKind kind;
    size_t packed_bytes;
    int out_channels;
    int padded_out_channels;
    int in_channels;
    int kernel_h;
    int kernel_w;
    int height;
    int width;
    int channels;
} PackedWeightDesc;

typedef struct WeightPackPlan {
    const NetGraph* graph;
    const LayoutPlan* layout_plan;
    PackedWeightDesc* node_descs;  /* Indexed by node id */
    int desc_count;
    int packed_node_count;
    size_t total_packed_bytes;
} WeightPackPlan;

WeightPackPlan* weight_pack_plan_build(const NetGraph* graph, const LayoutPlan* layout_plan);
void weight_pack_plan_free(WeightPackPlan* plan);
const PackedWeightDesc* weight_pack_plan_get_desc(const WeightPackPlan* plan, const GraphNode* node);

#endif /* NETLANG_WEIGHT_PACK_PLAN_H */
