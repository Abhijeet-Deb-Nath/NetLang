# Weight Packing

NetLang now uses a dedicated weight-packing analysis layer for inference-time specialization.

## Purpose

The goal is to trade extra initialization work and extra packed-weight memory for lower steady-state inference cost.

This is a backend optimization. It is not a language-level feature and it is not tied to a specific named architecture.

## Current Packed Paths

### Conv2D OC8 Packing

Supported Conv2D layers are repacked from the on-disk OIHW order into an output-channel-blocked layout with 8-channel AVX2-friendly groups.

That layout allows the hot convolution loop to:

- broadcast one input scalar
- load 8 output-channel weights contiguously
- accumulate 8 output channels with one FMA vector

This is the current main throughput optimization for CPU inference.

### Dense HWC Layout Specialization

When a 3D activation flows through `Flatten -> Dense`, and that flattened value is only consumed by dense layers, NetLang can:

- alias the flattened value to the source activation storage
- repack dense weights once at init
- remove the runtime HWC-to-CHW flatten materialization

That optimization is documented separately in [LAYOUT_SPECIALIZATION.md](LAYOUT_SPECIALIZATION.md).

## Current Tradeoff

Packed weights increase internal runtime memory use.

The benchmark harness reports this as:

- `repacked_weight_bytes`

That metric includes all runtime-generated packed-weight buffers for the compiled network.

## Current Scope

The current weight-packing plan covers:

- `Conv2D` via OC8-packed AVX2 kernels
- eligible `Dense` consumers of aliasable `Flatten` values

It does not yet cover:

- quantized layouts
- grouped or depthwise convolution
- wider autotuned blocking families
- target-specific multi-version kernel selection

## Architectural Boundary

The packing decision lives in the compiler analysis layer, not in ad hoc emitter branches.

That keeps:

- packing policy
- packed-memory accounting
- code generation

separate and easier to extend.
