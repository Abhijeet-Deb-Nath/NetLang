# NetLang Tools

Utility scripts for weight conversion, preprocessing, and testing.

---

## Weight Converters

**Convert PyTorch models:**
```bash
python tools/convert_pytorch.py model.pth output.nwf
pip install torch numpy
```

**Convert ONNX models:**
```bash
python tools/convert_onnx.py model.onnx output.nwf
pip install onnx numpy
```

**Convert Keras models:**
```bash
python tools/convert_keras.py model.h5 output.nwf
pip install tensorflow numpy
```

---

## Data Preprocessing

**Download MNIST dataset:**
```bash
python tools/download_mnist_for_netlang.py
# Downloads 10,000 preprocessed test images to test_data/
```

**Preprocess images:**
```bash
python tools/preprocess.py input.jpg output.bin
# Converts image to raw float32 binary format
```

---

## Testing & Benchmarking

**test_network.c** - C test harness for inference  
**benchmark.c** - Performance profiling  
**create_test_weights.c** - Generate random weights for testing

---

## Weight Format

All converters output NetLang `.nwf` format (binary weight files).  
See [docs/WEIGHT_FORMAT.md](../docs/WEIGHT_FORMAT.md) for specification.

**Format:** 64-byte aligned, little-endian, magic header `NWF0`
