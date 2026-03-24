#include "weight_pack_plan.h"
#include <stdlib.h>

static int round_up_to_8(int value) {
    return (value + 7) & ~7;
}

WeightPackPlan* weight_pack_plan_build(const NetGraph* graph, const LayoutPlan* layout_plan) {
    WeightPackPlan* plan = NULL;

    if (!graph) {
        return NULL;
    }

    plan = calloc(1, sizeof(WeightPackPlan));
    if (!plan) {
        return NULL;
    }

    plan->graph = graph;
    plan->layout_plan = layout_plan;
    plan->desc_count = graph->node_count;
    plan->node_descs = calloc((size_t)plan->desc_count, sizeof(PackedWeightDesc));
    if (!plan->node_descs) {
        weight_pack_plan_free(plan);
        return NULL;
    }

    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = graph->nodes[i];
        PackedWeightDesc* desc = &plan->node_descs[i];

        if (!node || !node->ast_node) {
            continue;
        }

        if (node->op_type == NODE_CONV2D && node->input_count == 1 && node->inputs[0] && node->inputs[0]->type) {
            int out_channels = node->ast_node->data.conv2d.filters;
            int padded_out_channels = round_up_to_8(out_channels);
            int in_channels = node->inputs[0]->type->dims[2];
            int kernel_h = node->ast_node->data.conv2d.kernel[0];
            int kernel_w = node->ast_node->data.conv2d.kernel[1];

            desc->kind = PACKED_WEIGHT_CONV_OC8;
            desc->out_channels = out_channels;
            desc->padded_out_channels = padded_out_channels;
            desc->in_channels = in_channels;
            desc->kernel_h = kernel_h;
            desc->kernel_w = kernel_w;
            desc->packed_bytes =
                (size_t)kernel_h *
                (size_t)kernel_w *
                (size_t)in_channels *
                (size_t)padded_out_channels *
                sizeof(float);
        } else if (node->op_type == NODE_DENSE) {
            const DenseLayoutTransform* transform = layout_plan_get_dense_transform(layout_plan, node);

            if (transform && transform->enabled) {
                int out_features = node->ast_node->data.dense.units;
                int in_features = transform->height * transform->width * transform->channels;

                desc->kind = PACKED_WEIGHT_DENSE_HWC;
                desc->height = transform->height;
                desc->width = transform->width;
                desc->channels = transform->channels;
                desc->packed_bytes = (size_t)out_features * (size_t)in_features * sizeof(float);
            }
        }

        if (desc->kind != PACKED_WEIGHT_NONE) {
            plan->packed_node_count++;
            plan->total_packed_bytes += desc->packed_bytes;
        }
    }

    return plan;
}

void weight_pack_plan_free(WeightPackPlan* plan) {
    if (!plan) {
        return;
    }

    free(plan->node_descs);
    free(plan);
}

const PackedWeightDesc* weight_pack_plan_get_desc(const WeightPackPlan* plan, const GraphNode* node) {
    if (!plan || !node || node->id < 0 || node->id >= plan->desc_count) {
        return NULL;
    }

    return &plan->node_descs[node->id];
}
