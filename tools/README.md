# Tools

Utility scripts and helper programs for conversion, preprocessing, and smoke testing.

## Conversion Tools

Current practical order of trust:

1. `convert_onnx.py`
2. `convert_pytorch.py`
3. `convert_keras.py`

The repo is being centered on compiler work first, so converter breadth is not the main research story.

## Data Helpers

- `download_mnist_for_netlang.py`
- `preprocess.py`
- `regenerate_mnist_28x28.py`

## Smoke-Test Helpers

- `test_network.c`
- `benchmark.c`
- `check_nwf.py`
- `convert_lenet_weights.py`

Current split:

- `test_network.c` is the MNIST-oriented correctness and smoke runner
- `benchmark.c` is the generic in-process benchmark harness for generated networks
- `run_eval.py` is the orchestration layer for compile timing, NetLang runtime timing, ONNX Runtime comparison, and JSON result export
- `run_eval_matrix.py` is the multi-sample evaluation runner for aggregated CSV/JSON summaries
- `legacy/` is reserved for intentionally retired helper scripts that are no longer part of the supported workflow

## Weight Files

All converters target `.nwf`.

See [Weight Format](../docs/WEIGHT_FORMAT.md) for the implementation-aligned format notes.
