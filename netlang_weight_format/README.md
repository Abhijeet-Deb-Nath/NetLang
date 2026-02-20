# Model Weights (models/)

This directory contains neural network weights in NetLang Weight Format (`.nwf`).

## NetLang Weight Format (.nwf)

A custom binary format optimized for:
- **Zero-copy mmap loading** (~1ms vs ~80ms for ONNX)
- **64-byte alignment** for AVX2 vectorized operations
- **Sequential memory layout** for cache efficiency
- **Direct pointer arithmetic** without parsing overhead

See [docs/WEIGHT_FORMAT.md](../docs/WEIGHT_FORMAT.md) for complete specification.

## Current Models

- `mnist_cnn.nwf` - LeNet-5 variant for MNIST digit classification (28×28 input)
- `lenet5.nwf` - Standard LeNet-5 architecture (32×32 input)

## Creating .nwf Files

Convert from popular frameworks:

### From PyTorch
```bash
python tools/convert_pytorch.py --model model.pth --output models/model.nwf
```

### From ONNX
```bash
python tools/convert_onnx.py --model model.onnx --output models/model.nwf
```

### From Keras/TensorFlow
```bash
python tools/convert_keras.py --model model.h5 --output models/model.nwf
```

## File Structure

```
Header (256 bytes)
├── Magic: "NWGT"
├── Version: 1
├── Layer count
├── Total size
└── Alignment: 64 bytes

Layer Metadata (64 bytes per layer)
├── Layer type (Conv2D/Dense/BatchNorm)
├── Weight offset
├── Weight shape [H, W, C_in, C_out]
└── Bias size

Weight Data (64-byte aligned)
├── Layer 0: weights + bias
├── Layer 1: weights + bias
└── ...
```

## Usage in NetLang

Reference weight files in your network definition:

```netlang
network MNIST_CNN {
    input(shape: [32, 32, 1])
    weights("models/lenet5.nwf")  // ← Points to this directory
    
    x = Conv2D(filters: 6, kernel: [5,5], activation: relu) from input
    // ... rest of network
}
```

## Performance Impact

Weight loading comparison:

| Format | Load Time | Notes |
|--------|-----------|-------|
| ONNX | 80ms | Requires parsing, no alignment |
| PyTorch .pth | 50ms | torch.load() overhead |
| .nwf (mmap) | **<1ms** | Memory-mapped, zero-copy |

## Notes

- Weight files can be large (VGG16: ~550MB, ResNet50: ~100MB)
- You may want to add `*.nwf` to `.gitignore` for large models
- Ensure your `.nlang` architecture matches the weight file structure
- Use `--verbose` flag on converters to verify layer extraction
