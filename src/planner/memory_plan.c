#include "memory_plan.h"
#include <stdlib.h>
#include <string.h>

#define SLOT_ALIGNMENT_ELEMENTS 16

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

static size_t align_elements(size_t value) {
    size_t mask = SLOT_ALIGNMENT_ELEMENTS - 1;
    return (value + mask) & ~mask;
}

static int compare_allocations(const void* lhs, const void* rhs) {
    const ValueAllocation* a = (const ValueAllocation*)lhs;
    const ValueAllocation* b = (const ValueAllocation*)rhs;

    if (a->first_step != b->first_step) {
        return a->first_step - b->first_step;
    }
    return a->value_id - b->value_id;
}

static int ensure_slot_capacity(MemoryPlan* plan, int needed_count) {
    if (needed_count <= plan->slot_capacity) {
        return 1;
    }

    int new_count = plan->slot_capacity == 0 ? 4 : plan->slot_capacity * 2;
    while (new_count < needed_count) {
        new_count *= 2;
    }

    MemorySlot* slots = realloc(plan->slots, (size_t)new_count * sizeof(MemorySlot));
    if (!slots) {
        return 0;
    }

    plan->slots = slots;
    plan->slot_capacity = new_count;
    return 1;
}

static int compute_last_use_step(const NetGraph* graph,
                                 const LayoutPlan* layout_plan,
                                 const GraphValue* value) {
    int last_step = -1;

    if (!graph || !value || !value->producer) {
        return last_step;
    }

    last_step = value->producer->topo_index;

    if (value == graph->output_value) {
        last_step = graph->topo_count;
    }

    for (int consumer_idx = 0; consumer_idx < value->consumer_count; consumer_idx++) {
        GraphNode* consumer = value->consumers[consumer_idx];
        if (consumer && consumer->topo_index > last_step) {
            last_step = consumer->topo_index;
        }
    }

    if (layout_plan) {
        for (int alias_value_id = 0; alias_value_id < graph->value_count; alias_value_id++) {
            GraphValue* alias_value = graph->values[alias_value_id];

            if (layout_plan->alias_source_ids[alias_value_id] != value->id || !alias_value) {
                continue;
            }

            if (alias_value == graph->output_value) {
                last_step = graph->topo_count;
            }

            for (int consumer_idx = 0; consumer_idx < alias_value->consumer_count; consumer_idx++) {
                GraphNode* consumer = alias_value->consumers[consumer_idx];
                if (consumer && consumer->topo_index > last_step) {
                    last_step = consumer->topo_index;
                }
            }
        }
    }

    return last_step;
}

MemoryPlan* memory_plan_build(const NetGraph* graph, const LayoutPlan* layout_plan) {
    MemoryPlan* plan = NULL;
    ValueAllocation* ordered = NULL;
    int ordered_count = 0;

    if (!graph) {
        return NULL;
    }

    plan = calloc(1, sizeof(MemoryPlan));
    if (!plan) {
        return NULL;
    }

    plan->graph = graph;
    plan->layout_plan = layout_plan;
    plan->allocation_count = graph->value_count;
    plan->allocations = calloc((size_t)plan->allocation_count, sizeof(ValueAllocation));
    if (!plan->allocations) {
        memory_plan_free(plan);
        return NULL;
    }

    ordered = calloc((size_t)graph->value_count, sizeof(ValueAllocation));
    if (!ordered) {
        memory_plan_free(plan);
        return NULL;
    }

    for (int i = 0; i < plan->allocation_count; i++) {
        plan->allocations[i].value_id = i;
        plan->allocations[i].slot_id = -1;
        plan->allocations[i].first_step = -1;
        plan->allocations[i].last_step = -1;
        plan->allocations[i].storage_kind = VALUE_STORAGE_EXTERNAL;
        plan->allocations[i].alias_value_id = -1;
    }

    for (int i = 0; i < graph->value_count; i++) {
        GraphValue* value = graph->values[i];
        ValueAllocation* allocation = &plan->allocations[value->id];
        int alias_source_id = layout_plan_get_alias_source_id(layout_plan, value);

        if (!value->producer) {
            continue;
        }

        allocation->element_count = tensor_element_count(value->type);

        if (alias_source_id >= 0) {
            allocation->storage_kind = VALUE_STORAGE_ALIAS;
            allocation->alias_value_id = alias_source_id;
            continue;
        }

        allocation->first_step = value->producer->topo_index;
        allocation->last_step = value->producer->topo_index;
        allocation->storage_kind = VALUE_STORAGE_SLOT;
        allocation->last_step = compute_last_use_step(graph, layout_plan, value);

        ordered[ordered_count++] = *allocation;
    }

    qsort(ordered, (size_t)ordered_count, sizeof(ValueAllocation), compare_allocations);

    for (int i = 0; i < ordered_count; i++) {
        ValueAllocation* ordered_alloc = &ordered[i];
        ValueAllocation* allocation = &plan->allocations[ordered_alloc->value_id];
        int selected_slot = -1;
        size_t selected_size = 0;

        for (int slot_idx = 0; slot_idx < plan->slot_count; slot_idx++) {
            MemorySlot* slot = &plan->slots[slot_idx];

            if (slot->last_step >= allocation->first_step) {
                continue;
            }

            if (slot->element_count < allocation->element_count) {
                continue;
            }

            if (selected_slot == -1 || slot->element_count < selected_size) {
                selected_slot = slot_idx;
                selected_size = slot->element_count;
            }
        }

        if (selected_slot == -1) {
            if (!ensure_slot_capacity(plan, plan->slot_count + 1)) {
                free(ordered);
                memory_plan_free(plan);
                return NULL;
            }

            selected_slot = plan->slot_count;
            plan->slots[selected_slot].id = selected_slot;
            plan->slots[selected_slot].element_count = allocation->element_count;
            plan->slots[selected_slot].last_step = allocation->last_step;
            plan->slots[selected_slot].offset_elements = 0;
            plan->slot_count++;
        } else {
            plan->slots[selected_slot].last_step = allocation->last_step;
        }

        allocation->slot_id = selected_slot;
    }

    free(ordered);

    size_t cursor = 0;
    for (int i = 0; i < plan->slot_count; i++) {
        MemorySlot* slot = &plan->slots[i];
        cursor = align_elements(cursor);
        slot->offset_elements = cursor;
        cursor += slot->element_count;
    }

    plan->arena_elements = cursor;
    plan->arena_bytes = cursor * sizeof(float);
    return plan;
}

void memory_plan_free(MemoryPlan* plan) {
    if (!plan) {
        return;
    }

    free(plan->allocations);
    free(plan->slots);
    free(plan);
}

const ValueAllocation* memory_plan_get_allocation(const MemoryPlan* plan, const GraphValue* value) {
    if (!plan || !value || value->id < 0 || value->id >= plan->allocation_count) {
        return NULL;
    }

    return &plan->allocations[value->id];
}

const ValueAllocation* memory_plan_get_storage_allocation(const MemoryPlan* plan, const GraphValue* value) {
    const GraphValue* current_value = value;
    int guard = 0;

    if (!plan || !value) {
        return NULL;
    }

    while (current_value && guard <= plan->allocation_count) {
        const ValueAllocation* allocation = memory_plan_get_allocation(plan, current_value);
        if (!allocation) {
            return NULL;
        }

        if (allocation->storage_kind != VALUE_STORAGE_ALIAS) {
            return allocation;
        }

        if (allocation->alias_value_id < 0 || allocation->alias_value_id >= plan->allocation_count) {
            return allocation;
        }

        current_value = plan->graph->values[allocation->alias_value_id];
        guard++;
    }

    return NULL;
}
