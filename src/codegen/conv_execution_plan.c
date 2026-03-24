#include "conv_execution_plan.h"
#include <stdlib.h>

static long long estimate_conv_work(const LayerInfo* layer, const PackedWeightDesc* packed_desc) {
    long long out_h = 0;
    long long out_w = 0;
    long long kernel_h = 0;
    long long kernel_w = 0;
    long long in_channels = 0;
    long long out_channels = 0;

    if (!layer || !layer->layer_node || !layer->input_shape || !layer->output_shape) {
        return 0;
    }

    out_h = layer->output_shape->dims[0];
    out_w = layer->output_shape->dims[1];
    kernel_h = layer->layer_node->data.conv2d.kernel[0];
    kernel_w = layer->layer_node->data.conv2d.kernel[1];
    in_channels = layer->input_shape->dims[2];
    out_channels = packed_desc ? packed_desc->padded_out_channels : layer->layer_node->data.conv2d.filters;

    return out_h * out_w * kernel_h * kernel_w * in_channels * out_channels;
}

static int choose_spatial_block_width(const LayerInfo* layer, const PackedWeightDesc* packed_desc) {
    int stride = 0;
    int kernel_w = 0;
    int out_w = 0;
    int in_channels = 0;
    long long work = 0;
    long long reuse_score = 0;

    if (!layer || layer->layer_type != NODE_CONV2D || !layer->layer_node) {
        return 1;
    }

    if (!packed_desc || packed_desc->kind != PACKED_WEIGHT_CONV_OC8) {
        return 1;
    }

    if (!layer->input_shape || !layer->output_shape) {
        return 1;
    }

    stride = layer->layer_node->data.conv2d.stride;
    kernel_w = layer->layer_node->data.conv2d.kernel[1];
    out_w = layer->output_shape->dims[1];
    in_channels = layer->input_shape->dims[2];
    work = estimate_conv_work(layer, packed_desc);
    reuse_score = (long long)kernel_w * in_channels * packed_desc->padded_out_channels;

    if (stride > 2 || out_w <= 1) {
        return 1;
    }

    /*
     * Choose a small output-width micro-tile only when the layer has enough
     * total work and a compact enough sliding window to reuse the loaded
     * packed weights across adjacent outputs.
     */
    if (out_w >= 4 &&
        work >= 50000 &&
        ((4 - 1) * stride + kernel_w) <= 16 &&
        reuse_score >= 256) {
        return 4;
    }

    if (out_w >= 2 &&
        work >= 15000 &&
        ((2 - 1) * stride + kernel_w) <= 12 &&
        reuse_score >= 128) {
        return 2;
    }

    return 1;
}

ConvExecutionPlan* conv_execution_plan_build(CodegenContext* ctx) {
    ConvExecutionPlan* plan = NULL;

    if (!ctx || !ctx->layers || ctx->layer_count <= 0) {
        return NULL;
    }

    plan = calloc(1, sizeof(ConvExecutionPlan));
    if (!plan) {
        return NULL;
    }

    plan->layer_count = ctx->layer_count;
    plan->layer_choices = calloc((size_t)plan->layer_count, sizeof(ConvExecutionChoice));
    if (!plan->layer_choices) {
        conv_execution_plan_free(plan);
        return NULL;
    }

    for (int i = 0; i < ctx->layer_count; i++) {
        LayerInfo* layer = &ctx->layers[i];
        const PackedWeightDesc* packed_desc = NULL;

        plan->layer_choices[i].spatial_block_width = 1;

        if (ctx->weight_plan && layer->graph_node) {
            packed_desc = weight_pack_plan_get_desc(ctx->weight_plan, layer->graph_node);
        }

        plan->layer_choices[i].spatial_block_width = choose_spatial_block_width(layer, packed_desc);
    }

    return plan;
}

void conv_execution_plan_free(ConvExecutionPlan* plan) {
    if (!plan) {
        return;
    }

    free(plan->layer_choices);
    free(plan);
}

const ConvExecutionChoice* conv_execution_plan_get_choice(const ConvExecutionPlan* plan, int layer_index) {
    if (!plan || layer_index < 0 || layer_index >= plan->layer_count) {
        return NULL;
    }

    return &plan->layer_choices[layer_index];
}
