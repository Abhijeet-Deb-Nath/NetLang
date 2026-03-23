# Assets

This directory contains non-source project assets used by examples and evaluation.

## Layout

- `weights/netlang/` stores `.nwf` weight files used by NetLang examples
- `models/onnx/` stores baseline ONNX models used for parity and evaluation
- `inputs/preprocessed_28x28/` stores ready-to-run float32 input tensors
- `inputs/raw/` is the staging area for optional image preprocessing
- `datasets/` stores downloaded dataset caches and raw dataset assets

These files support the compiler and experiments, but they are not part of the compiler implementation itself.
