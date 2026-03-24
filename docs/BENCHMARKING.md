# Benchmarking

NetLang should not publish timing claims from ad hoc scripts.

This document defines the benchmark protocol for the current research target.

## Separate These Stages

The evaluation must report them separately:

1. compile time
2. load and initialization time
3. first inference latency
4. warm steady-state inference latency
5. peak memory
6. generated artifact size

If those are mixed together, the result is not useful.

## Current Repository Reality

The current harnesses are for smoke testing, not final evaluation.

In particular:

- `tools/test_network.c` is a correctness-oriented runner
- `tools/benchmark.c` is now the dedicated in-process benchmark harness
- current quick timing inside the smoke harness is rough only
- subprocess-heavy Python wrappers are not the final comparison method

## Benchmark Rules

- keep compilation outside warm inference measurements
- keep model load outside warm inference measurements
- use repeated in-process inference for warm runs
- report mean, median, and tail behavior
- fix thread count and machine settings for all baselines
- report the selected NetLang thread count in every published run
- compare the same model, weights, precision, and preprocessing path

## Target Metrics

Primary:

- peak activation memory
- first inference latency
- warm batch-1 latency
- binary size

The new planner phase makes peak activation memory a real compiler metric instead of a hand-wavy future claim, and the current benchmark harness can now report:

- activation arena bytes
- reusable slot count
- repacked weight bytes
- runtime thread count
- init time
- first inference latency
- warm mean / median / p95 latency
- executable size

Secondary:

- compile time
- weight-load overhead
- total resident memory

## Baseline Philosophy

The goal is not to compare against raw Python execution.

The goal is to compare against compiled or runtime-managed inference stacks fairly and narrowly for the deployment regime this repo cares about.

## Current Gap

The current benchmark harness is the repo's internal runtime-measurement path. It still does not solve:

- compile-time measurement
- baseline orchestration across external runtimes
- full peak resident-memory measurement

## Automation Path

The repo now has an evaluation driver:

```powershell
python tools\run_eval.py ^
  --network examples\lenet5.nlang ^
  --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin ^
  --onnx assets\models\onnx\lenet_mnist.onnx ^
  --warmup 25 ^
  --runs 200
```

That driver orchestrates:

- NetLang code generation time
- native benchmark executable build time
- NetLang in-process runtime metrics
- ONNX Runtime baseline metrics
- output comparison on the same input sample

The current gap after this step is broader multi-model and multi-runtime result collection, not single-model measurement plumbing.

## Thread Control

NetLang runtime thread count is selected at runtime. By default it uses the
machine-derived thread count exposed by the generated network metadata. For
controlled experiments, set `NETLANG_THREADS` before launching the benchmark or
evaluation driver.

Example:

```powershell
$env:NETLANG_THREADS='1'
python tools\run_eval.py ^
  --network examples\lenet5.nlang ^
  --input assets\inputs\preprocessed_28x28\mnist_0000_label_7.bin ^
  --onnx assets\models\onnx\lenet_mnist.onnx
Remove-Item Env:NETLANG_THREADS
```

## Matrix Path

The repo now also has a multi-sample matrix runner:

```powershell
python tools\run_eval_matrix.py ^
  --network examples\lenet5.nlang ^
  --input-glob assets\inputs\preprocessed_28x28\*.bin ^
  --onnx assets\models\onnx\lenet_mnist.onnx ^
  --warmup 5 ^
  --runs 20 ^
  --sample-limit 25
```

That path:

- builds the NetLang artifact once
- reuses the benchmark executable across many samples
- writes per-sample CSV results
- writes an aggregate JSON summary with accuracy, latency, and parity metrics

The current gap after this step is broader multi-model experimentation and stronger baseline coverage, not reference-path measurement plumbing.
