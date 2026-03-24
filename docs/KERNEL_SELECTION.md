# Kernel Selection

Kernel selection is now an explicit backend policy layer.

## What It Is

The compiler chooses a kernel family from:

- operator kind
- activation pattern
- packed-weight availability
- blocking metadata
- runtime execution policy eligibility

It does **not** choose kernels by architecture name or model name.

That means it is a general backend policy, not an architecture-specific hack.

## Current Policy Surface

### Conv2D

Current Conv2D choices include:

- legacy direct kernel
- fused ReLU kernel
- blocked kernel
- packed OC8 kernel
- packed OC8 + blocked kernel
- fused variants of the packed kernels

### Dense

Current Dense choices include:

- base dense kernel
- fused ReLU dense kernel
- HWC-specialized packed dense path for eligible `Flatten -> Dense`

## Why This Exists

Without an explicit selection layer, code generation drifts into emitter-local branching:

- harder to read
- harder to test
- harder to extend
- easier to accidentally turn into a pile of hacks

The selection layer keeps backend policy separate from:

- graph lowering
- memory planning
- runtime execution planning
- emitted C syntax

## Current Boundary

Today the kernel choice is still heuristic and CPU-target-specific.

That is acceptable.

The current selection layer is paired with a runtime execution backend that can
parallelize eligible packed Conv2D kernels over output-channel blocks and pair
them with an output-width micro-tile plan. Those decisions are shape-driven and
thread-count-driven, not
architecture-name-driven.

What would be a hack is:

- selecting on model names
- selecting on hand-curated architecture labels
- adding one-off emitter branches for special benchmark networks

That is explicitly not the direction here.
