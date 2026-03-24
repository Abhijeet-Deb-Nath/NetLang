# NetLang

NetLang is a research compiler prototype for fixed-shape CNN inference on CPU.

The repo is now centered on one stable mainline:

> a restricted CNN compiler that lowers fixed-shape networks to C and evaluates how far a packed, thread-aware CPU backend can be pushed without architecture-specific hacks

That is the current codebase message. Experimental region-based tiling work has
been removed from mainline because it added complexity without improving the
reference path.

## Current Reality

What works today:

- lexing, parsing, and semantic checks for `.nlang` network files
- external weight declarations via `.nwf`
- graph IR lowering for the generated op subset
- planner-backed static activation arena with slot reuse
- ahead-of-time C code generation

What is stable for generated execution today:

- `Conv2D`
- `Dense`
- `MaxPool`
- `AvgPool`
- `Flatten`
- OC8-packed Conv2D weights
- packed Conv2D AVX2/FMA kernels
- output-width micro-tiling inside packed Conv2D
- layout-specialized `Flatten -> Dense` lowering with repacked dense weights
- explicit kernel selection from shape/layout facts
- persistent runtime threadpool for eligible packed Conv2D layers
- generated metadata for arena size, slot count, repacked weight bytes, and thread count

What exists but should still be treated cautiously:

- `Add` and `Concat` for fixed-shape graph experiments
- graph-aware lowering and topological scheduling outside the reference sequential path

What is not yet an honest end-to-end claim:

- broad modern CNN coverage
- general DAG performance claims
- oneDNN/ONNX Runtime parity on warm inference
- publication-grade multi-model evaluation

## Current Reference Result

The stable reference backend is the packed-spatial CPU path preserved in
`results/evaluation/lenet5_stable_matrix_25.json`.

On the current 25-sample LeNet reference matrix:

- NetLang warm-mean average: `1.002 ms`
- ONNX Runtime warm-mean average: `0.240 ms`
- accuracy: `25/25`
- argmax match rate: `1.0`

That is the honest mainline position today: roughly `4.2x` slower than the
local ONNX Runtime CPU baseline on the reference workload.

## Research Target

NetLang is still being shaped around a constrained target system:

- fixed-shape CNN inference
- ahead-of-time lowering to C
- explicit graph IR
- compile-time activation planning
- packed and shape-specialized CPU execution
- honest CPU-side evaluation

The repo should not claim more than that until the measurements justify it.

## Repo Guide

- [Quick Start](QUICKSTART.md)
- [Roadmap](docs/ROADMAP.md)
- [Current Status](docs/STATUS.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Graph IR](docs/GRAPH_IR.md)
- [Memory Planning](docs/MEMORY_PLANNING.md)
- [Layout Specialization](docs/LAYOUT_SPECIALIZATION.md)
- [Weight Packing](docs/WEIGHT_PACKING.md)
- [Conv Execution Plan](docs/CONV_EXECUTION_PLAN.md)
- [Kernel Selection](docs/KERNEL_SELECTION.md)
- [Execution Backend](docs/EXECUTION_BACKEND.md)
- [Syntax](docs/SYNTAX.md)
- [Benchmarking](docs/BENCHMARKING.md)
- [Weight Format](docs/WEIGHT_FORMAT.md)
- [Examples](examples/README.md)

## Quick Start

Build the compiler:

```powershell
build.bat
```

Compile the reference sequential example:

```powershell
build\netlang.exe examples\lenet5.nlang -o generated\lenet5.c
build_network.bat generated\lenet5.c lenet5_smoke
bin\lenet5_smoke.exe assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin
```

This smoke path is useful for correctness checks. It is not the final benchmark methodology for the research target.

Build the in-process benchmark harness:

```powershell
build_benchmark.bat generated\lenet5.c lenet5_bench
bin\lenet5_bench.exe --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin --warmup 25 --runs 200
```

Override the runtime thread count if you need controlled comparisons:

```powershell
$env:NETLANG_THREADS='1'
bin\lenet5_bench.exe --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin --warmup 25 --runs 200
Remove-Item Env:NETLANG_THREADS
```

Run the end-to-end evaluation driver:

```powershell
python tools\run_eval.py ^
  --network examples\lenet5.nlang ^
  --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin ^
  --onnx assets\models\onnx\lenet_mnist.onnx ^
  --warmup 25 ^
  --runs 200
```

Run the multi-sample matrix:

```powershell
python tools\run_eval_matrix.py ^
  --network examples\lenet5.nlang ^
  --input-glob assets\inputs\preprocessed_28x28\*.bin ^
  --onnx assets\models\onnx\lenet_mnist.onnx ^
  --warmup 5 ^
  --runs 20 ^
  --sample-limit 25
```

## Repository Layout

```text
assets/         sample inputs, weights, ONNX models, and dataset assets
fixtures/       parser fixtures and legacy internal network inputs
src/            compiler implementation
examples/       public example networks
tools/          converters, preprocessing, and smoke-test helpers
generated/      generated C output
bin/            built executables and DLLs
docs/           research and implementation docs
results/        evaluation outputs
```

## Ground Rules

- parser support is not the same as codegen support
- performance claims must follow `docs/BENCHMARKING.md`
- public docs should describe the codegen-ready subset first
- do not keep experimental branches in mainline after they stop paying for their complexity
