# Execution Backend

This document describes the current runtime execution model for generated
NetLang networks.

## Principle

Execution policy is allowed to depend on:

- operator kind
- tensor shape
- storage layout
- packed-weight availability
- runtime thread count
- target CPU capabilities

Execution policy is **not** allowed to depend on:

- model name
- benchmark name
- hand-curated architecture labels

That boundary is what keeps the backend general instead of turning into a pile
of architecture-specific fast paths.

## Current Runtime Model

Generated networks currently execute with:

- a static activation arena
- late-bound external weights
- optional runtime-repacked packed weights
- a persistent runtime threadpool
- per-op kernel dispatch through the generated backend policy

The runtime threadpool is created once in `network_init()` and destroyed in
`network_cleanup()`. It is then reused across repeated inference calls.

## Current Parallel Path

Today the general parallel path is limited to eligible packed `Conv2D` kernels.

The current implementation:

- uses OC8-packed weights
- vectorizes over 8 output channels with AVX2/FMA
- reuses each loaded packed weight vector across a small output-width micro-tile
- parallelizes work over output-channel block ranges
- falls back to serial execution for small layers or single-thread runs

This is a general operator/backend policy. It is not tied to LeNet, VGG, ResNet,
or any other named architecture.

## Thread Count

Generated networks expose:

- default runtime thread count
- actual runtime thread count for the initialized network

The runtime also honors the `NETLANG_THREADS` environment variable for
controlled experiments.

For controlled backend-mode recovery experiments, the runtime also honors
`NETLANG_CONV_SPATIAL_BLOCK`:

- `auto` or unset: use the generated execution plan
- `1`: force `OWx1`
- `2`: force `OWx2`
- `4`: force `OWx4`

Example:

```powershell
$env:NETLANG_THREADS='1'
$env:NETLANG_CONV_SPATIAL_BLOCK='1'
bin\lenet5_bench.exe --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin --warmup 25 --runs 200
Remove-Item Env:NETLANG_THREADS
Remove-Item Env:NETLANG_CONV_SPATIAL_BLOCK
```

## Current Limitation

The current backend is still **per-op parallel**, not **cross-op scheduled**.

That means the system can:

- choose a kernel family for an op
- run eligible packed Conv2D kernels in parallel

But it does not yet:

- tile across multiple ops
- schedule branches with a global traffic objective
- pipeline producer and consumer tiles through the whole graph

Those broader execution strategies are the next general optimization direction
if the project continues to prioritize inference-time improvement.

## Current Evidence

The latest reference evaluation artifacts are:

- `results/evaluation/lenet5_stable_matrix_25.json`
- `results/evaluation/lenet5_stable_matrix_t1_25.json`

On that 25-sample LeNet reference matrix:

- default runtime threading averaged about `0.602 ms`
- forced single-thread averaged about `0.726 ms`

So the current packed-spatial execution backend is materially useful, but it
still does not beat the current local ONNX Runtime CPU baseline on the same
workload.
