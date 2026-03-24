#include "kernel_selection.h"
#include "blocking_config.h"
#include <stdlib.h>

KernelPlan* kernel_plan_build(CodegenContext* ctx) {
    KernelPlan* plan = NULL;

    if (!ctx || !ctx->layers || ctx->layer_count <= 0) {
        return NULL;
    }

    plan = calloc(1, sizeof(KernelPlan));
    if (!plan) {
        return NULL;
    }

    plan->layer_count = ctx->layer_count;
    plan->layer_choices = calloc((size_t)plan->layer_count, sizeof(KernelChoice));
    if (!plan->layer_choices) {
        kernel_plan_free(plan);
        return NULL;
    }

    for (int i = 0; i < ctx->layer_count; i++) {
        LayerInfo* layer = &ctx->layers[i];
        const PackedWeightDesc* packed_desc = NULL;
        KernelChoice* choice = &plan->layer_choices[i];

        choice->layer_type = layer->layer_type;

        if (ctx->weight_plan && layer->graph_node) {
            packed_desc = weight_pack_plan_get_desc(ctx->weight_plan, layer->graph_node);
        }

        if (layer->layer_type == NODE_CONV2D) {
            int use_packed = packed_desc && packed_desc->kind == PACKED_WEIGHT_CONV_OC8;
            int use_blocked = layer->layer_node &&
                              layer->layer_node->blocking_info &&
                              layer->layer_node->blocking_info->l1_tile_size > 0;
            int use_relu_fusion = layer->layer_node &&
                                  layer->layer_node->data.conv2d.activation == ACT_RELU;

            if (use_packed) {
                if (use_relu_fusion && use_blocked) choice->conv_kind = CONV_KERNEL_PACKED_OC8_FUSED_BLOCKED;
                else if (use_relu_fusion) choice->conv_kind = CONV_KERNEL_PACKED_OC8_FUSED;
                else if (use_blocked) choice->conv_kind = CONV_KERNEL_PACKED_OC8_BLOCKED;
                else choice->conv_kind = CONV_KERNEL_PACKED_OC8;
            } else {
                if (use_relu_fusion && use_blocked) choice->conv_kind = CONV_KERNEL_FUSED_BLOCKED;
                else if (use_relu_fusion) choice->conv_kind = CONV_KERNEL_FUSED;
                else if (use_blocked) choice->conv_kind = CONV_KERNEL_BLOCKED;
                else choice->conv_kind = CONV_KERNEL_LEGACY;
            }
        } else if (layer->layer_type == NODE_DENSE) {
            int use_packed = packed_desc && packed_desc->kind == PACKED_WEIGHT_DENSE_HWC;
            int use_relu_fusion = layer->layer_node &&
                                  layer->layer_node->data.dense.activation == ACT_RELU;

            if (use_packed && use_relu_fusion) choice->dense_kind = DENSE_KERNEL_PACKED_HWC_FUSED;
            else if (use_packed) choice->dense_kind = DENSE_KERNEL_PACKED_HWC;
            else if (use_relu_fusion) choice->dense_kind = DENSE_KERNEL_FUSED;
            else choice->dense_kind = DENSE_KERNEL_BASE;
        }
    }

    return plan;
}

void kernel_plan_free(KernelPlan* plan) {
    if (!plan) {
        return;
    }

    free(plan->layer_choices);
    free(plan);
}

const KernelChoice* kernel_plan_get_choice(const KernelPlan* plan, int layer_index) {
    if (!plan || layer_index < 0 || layer_index >= plan->layer_count) {
        return NULL;
    }

    return &plan->layer_choices[layer_index];
}
