#ifndef NETLANG_KERNEL_SELECTION_H
#define NETLANG_KERNEL_SELECTION_H

#include "codegen.h"

typedef enum ConvKernelKind {
    CONV_KERNEL_LEGACY = 0,
    CONV_KERNEL_FUSED,
    CONV_KERNEL_BLOCKED,
    CONV_KERNEL_FUSED_BLOCKED,
    CONV_KERNEL_PACKED_OC8,
    CONV_KERNEL_PACKED_OC8_FUSED,
    CONV_KERNEL_PACKED_OC8_BLOCKED,
    CONV_KERNEL_PACKED_OC8_FUSED_BLOCKED
} ConvKernelKind;

typedef enum DenseKernelKind {
    DENSE_KERNEL_BASE = 0,
    DENSE_KERNEL_FUSED,
    DENSE_KERNEL_PACKED_HWC,
    DENSE_KERNEL_PACKED_HWC_FUSED
} DenseKernelKind;

typedef struct KernelChoice {
    NodeType layer_type;
    ConvKernelKind conv_kind;
    DenseKernelKind dense_kind;
} KernelChoice;

typedef struct KernelPlan {
    KernelChoice* layer_choices; /* Indexed by LayerInfo array index */
    int layer_count;
} KernelPlan;

KernelPlan* kernel_plan_build(CodegenContext* ctx);
void kernel_plan_free(KernelPlan* plan);
const KernelChoice* kernel_plan_get_choice(const KernelPlan* plan, int layer_index);

#endif /* NETLANG_KERNEL_SELECTION_H */
