#include "layout_plan.h"
#include <stdlib.h>

static size_t tensor_element_count(const TensorType* type) {
    size_t count = 1;

    if (!type) {
        return 0;
    }

    for (int i = 0; i < type->rank; i++) {
        count *= (size_t)type->dims[i];
    }

    return count;
}

LayoutPlan* layout_plan_build(const NetGraph* graph) {
    LayoutPlan* plan = NULL;

    if (!graph) {
        return NULL;
    }

    plan = calloc(1, sizeof(LayoutPlan));
    if (!plan) {
        return NULL;
    }

    plan->graph = graph;
    plan->alias_source_ids = malloc((size_t)graph->value_count * sizeof(int));
    plan->dense_transforms = calloc((size_t)graph->node_count, sizeof(DenseLayoutTransform));
    if (!plan->alias_source_ids || !plan->dense_transforms) {
        layout_plan_free(plan);
        return NULL;
    }

    for (int i = 0; i < graph->value_count; i++) {
        plan->alias_source_ids[i] = -1;
    }

    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = graph->nodes[i];
        GraphValue* flattened = NULL;
        GraphValue* source = NULL;
        TensorType* source_type = NULL;
        int can_alias = 1;

        if (!node || node->op_type != NODE_FLATTEN || node->input_count != 1 || !node->output) {
            continue;
        }

        flattened = node->output;
        source = node->inputs[0];
        source_type = source ? source->type : NULL;

        if (!source_type || source_type->rank != 3) {
            continue;
        }

        if (flattened == graph->output_value || flattened->consumer_count == 0) {
            continue;
        }

        for (int consumer_idx = 0; consumer_idx < flattened->consumer_count; consumer_idx++) {
            GraphNode* consumer = flattened->consumers[consumer_idx];
            if (!consumer || consumer->op_type != NODE_DENSE) {
                can_alias = 0;
                break;
            }
        }

        if (!can_alias) {
            continue;
        }

        if (plan->alias_source_ids[flattened->id] < 0) {
            plan->alias_source_ids[flattened->id] = source->id;
            plan->alias_value_count++;
        }

        for (int consumer_idx = 0; consumer_idx < flattened->consumer_count; consumer_idx++) {
            GraphNode* consumer = flattened->consumers[consumer_idx];
            DenseLayoutTransform* transform = &plan->dense_transforms[consumer->id];

            if (transform->enabled) {
                continue;
            }

            transform->enabled = 1;
            transform->flatten_value_id = flattened->id;
            transform->source_value_id = source->id;
            transform->height = source_type->dims[0];
            transform->width = source_type->dims[1];
            transform->channels = source_type->dims[2];
            plan->transformed_dense_count++;
            plan->repacked_weight_bytes +=
                tensor_element_count(flattened->type) *
                (size_t)consumer->ast_node->data.dense.units *
                sizeof(float);
        }
    }

    return plan;
}

void layout_plan_free(LayoutPlan* plan) {
    if (!plan) {
        return;
    }

    free(plan->alias_source_ids);
    free(plan->dense_transforms);
    free(plan);
}

int layout_plan_get_alias_source_id(const LayoutPlan* plan, const GraphValue* value) {
    if (!plan || !value || value->id < 0 || value->id >= plan->graph->value_count) {
        return -1;
    }

    return plan->alias_source_ids[value->id];
}

const GraphValue* layout_plan_get_alias_source(const LayoutPlan* plan, const GraphValue* value) {
    int source_id = layout_plan_get_alias_source_id(plan, value);

    if (source_id < 0) {
        return NULL;
    }

    return plan->graph->values[source_id];
}

const DenseLayoutTransform* layout_plan_get_dense_transform(const LayoutPlan* plan, const GraphNode* node) {
    if (!plan || !node || node->id < 0 || node->id >= plan->graph->node_count) {
        return NULL;
    }

    return &plan->dense_transforms[node->id];
}
