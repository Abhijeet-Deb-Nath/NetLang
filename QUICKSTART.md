# Quick Start

This quick start verifies the current compiler pipeline on the stable sequential subset.

It does not validate the full research target yet.

## Prerequisites

- Windows with GCC, Flex, and Bison available to `build.bat`
- or Linux/macOS with `make`, GCC/Clang, Flex, and Bison
- Python is optional unless you need data conversion tools

## 1. Build the Compiler

```powershell
build.bat
```

On Unix-like systems:

```bash
make
```

## 2. Generate C for the Reference Network

```powershell
build\netlang.exe examples\lenet5.nlang -o generated\lenet5.c
```

This example is the current reference path because it stays inside the sequential codegen-ready subset.

## 3. Build a Smoke-Test Executable

```powershell
build_network.bat generated\lenet5.c lenet5_smoke
```

This script links the generated file with:

- `src/codegen/runtime.c`
- `src/codegen/kernels.c`
- `tools/test_network.c`

## 4. Run a Sample Inference

```powershell
bin\lenet5_smoke.exe assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin
```

The current harness prints a predicted class and a rough elapsed time.

That elapsed time is only a smoke-test signal. It is not paper-grade benchmarking.

## 5. Build The Benchmark Harness

```powershell
build_benchmark.bat generated\lenet5.c lenet5_bench
bin\lenet5_bench.exe --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin --warmup 25 --runs 200
```

This benchmark path reports:

- init time
- first inference latency
- warm in-process latency statistics
- activation arena bytes
- reusable slot count
- runtime thread count
- executable size

The generated runtime creates a persistent threadpool by default. You can force a
specific thread count for controlled comparisons with `NETLANG_THREADS`:

```powershell
$env:NETLANG_THREADS='1'
bin\lenet5_bench.exe --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin --warmup 25 --runs 200
Remove-Item Env:NETLANG_THREADS
```

## 6. Run The End-To-End Evaluation Driver

```powershell
python tools\run_eval.py ^
  --network examples\lenet5.nlang ^
  --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin ^
  --onnx assets\models\onnx\lenet_mnist.onnx ^
  --warmup 25 ^
  --runs 200
```

This produces a combined JSON report under `results/evaluation/`.

## 7. Run The Multi-Sample Matrix

```powershell
python tools\run_eval_matrix.py ^
  --network examples\lenet5.nlang ^
  --input-glob assets\inputs\preprocessed_28x28\*.bin ^
  --onnx assets\models\onnx\lenet_mnist.onnx ^
  --warmup 5 ^
  --runs 20 ^
  --sample-limit 25
```

This produces:

- an aggregate JSON summary
- a per-sample CSV table
- reusable benchmark artifacts for the evaluated network

## Where To Go Next

- [Roadmap](docs/ROADMAP.md)
- [Current Status](docs/STATUS.md)
- [Execution Backend](docs/EXECUTION_BACKEND.md)
- [Benchmarking](docs/BENCHMARKING.md)
- [Syntax](docs/SYNTAX.md)
