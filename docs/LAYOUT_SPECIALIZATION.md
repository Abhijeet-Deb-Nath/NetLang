# Layout Specialization

NetLang now includes a general layout-contract optimization for eligible `Flatten -> Dense` boundaries.

## What It Does

When the compiler sees:

- a 3D producer tensor in native HWC storage
- a `Flatten`
- one or more `Dense` consumers
- no non-`Dense` consumers of that flattened value

it can treat the `Flatten` as a view rather than a materialized tensor.

Instead of writing a CHW-flattened buffer at inference time, the compiler:

1. aliases the `Flatten` result to the source activation storage
2. repacks the affected dense-layer weight rows once during `network_init()`
3. runs the dense layer directly against the original HWC activation memory

## Why This Is General

This is not an architecture-specific fast path.

It is a structural rule over a graph pattern:

- source tensor rank
- consumer op type
- storage layout contract

Any supported fixed-shape CNN graph that satisfies that rule can use it.

## What It Buys

- removes one activation materialization step
- avoids a runtime HWC-to-CHW reorder
- keeps planner-visible storage reuse honest through alias-aware lifetimes

## What It Costs

- extra repacked dense-weight buffers allocated during initialization
- one-time init work for the repack

The benchmark harness now reports `repacked_weight_bytes` so the tradeoff stays visible.

## Current Scope

Today this optimization targets:

- 3D HWC activations
- `Flatten`
- `Dense`

It does not yet generalize to arbitrary multi-op tiling or broader view semantics.
