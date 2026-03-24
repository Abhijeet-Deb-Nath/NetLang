#ifndef NETLANG_MEMORY_PLAN_H
#define NETLANG_MEMORY_PLAN_H

#include "../graph/graph.h"
#include "layout_plan.h"
#include <stddef.h>

typedef enum ValueStorageKind {
    VALUE_STORAGE_EXTERNAL = 0,
    VALUE_STORAGE_SLOT = 1,
    VALUE_STORAGE_ALIAS = 2
} ValueStorageKind;

typedef struct ValueAllocation {
    int value_id;
    int slot_id;            /* -1 for external values such as network input */
    int first_step;
    int last_step;
    size_t element_count;
    ValueStorageKind storage_kind;
    int alias_value_id;     /* Valid when storage_kind == VALUE_STORAGE_ALIAS */
} ValueAllocation;

typedef struct MemorySlot {
    int id;
    int last_step;
    size_t element_count;
    size_t offset_elements;
} MemorySlot;

typedef struct MemoryPlan {
    const NetGraph* graph;
    const LayoutPlan* layout_plan;
    ValueAllocation* allocations;
    int allocation_count;
    MemorySlot* slots;
    int slot_count;
    int slot_capacity;
    size_t arena_elements;
    size_t arena_bytes;
} MemoryPlan;

MemoryPlan* memory_plan_build(const NetGraph* graph, const LayoutPlan* layout_plan);
void memory_plan_free(MemoryPlan* plan);
const ValueAllocation* memory_plan_get_allocation(const MemoryPlan* plan, const GraphValue* value);
const ValueAllocation* memory_plan_get_storage_allocation(const MemoryPlan* plan, const GraphValue* value);

#endif /* NETLANG_MEMORY_PLAN_H */
