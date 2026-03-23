# NetLang

NetLang is a research compiler prototype for fixed-shape CNN inference.

The repo is now centered on one thesis:

> build a static CNN compiler that lowers a restricted network description to C, then evolve it into a true DAG compiler with compile-time activation memory planning and low-overhead deployment

This is the current research direction. It is the only direction this repo should optimize for.

## Current Reality

What works today:

- lexing, parsing, and semantic checks for `.nlang` network files
- weight-file declarations via `.nwf`
- C code generation for a sequential subset
- runtime support for external weights and aligned activation buffers

What is stable for code generation today:

- `Conv2D`
- `Dense`
- `MaxPool`
- `AvgPool`
- `Flatten`
- graph lowering with explicit value dependencies
- planner-backed static activation arena
- generated planner metadata for arena size and slot count

What is available but still experimental:

- `Add` for residual-style branch merge
- `Concat` for branch merge in fixed-shape graphs

What is not yet an honest end-to-end feature:

- general DAG execution
- detailed planner reporting and final benchmark harness integration
- publication-grade benchmarking

The repo previously mixed assignment-era documentation, inflated optimization claims, and parser-only features with actual compiler support. That drift has been removed from the top-level docs.

## Research Target

NetLang is being reorganized around this target system:

- fixed-shape CNN graphs
- ahead-of-time lowering to C
- explicit graph IR
- topological scheduling
- compile-time activation lifetime analysis
- static arena allocation and buffer reuse
- late-bound external weights
- deployment evaluation on CPU and edge-like constraints

The goal is not "beat every existing runtime on generic throughput."

The goal is:

- lower overhead deployment
- stronger resource predictability
- compile-time memory planning
- honest, narrow compiler contributions

## Repo Guide

- [Quick Start](QUICKSTART.md)
- [Roadmap](docs/ROADMAP.md)
- [Current Status](docs/STATUS.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Graph IR](docs/GRAPH_IR.md)
- [Memory Planning](docs/MEMORY_PLANNING.md)
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
- new language features are secondary to graph IR, scheduling, and memory planning
