# NetLang Tools

Utility scripts for weight conversion, data preprocessing, testing, and benchmarking.

---

## Quick Reference

| Tool | Purpose | Input | Output |
|------|---------|-------|--------|
| [convert_pytorch.py](#pytorch-converter) | PyTorch → .nwf | `.pth` / `.pt` | `.nwf` |
| [convert_keras.py](#keras-converter) | Keras/TF → .nwf | `.h5` / SavedModel | `.nwf` |
| [convert_onnx.py](#onnx-converter) | ONNX → .nwf | `.onnx` | `.nwf` |
| [preprocess.py](#image-preprocessor) | Image → binary | `.jpg/.png` | `.bin` |
| [download_mnist_for_netlang.py](#mnist-downloader) | Download MNIST | - | 10,000 `.bin` files |
| [test_network.c](#test-harness) | C inference tester | `.bin` + `.nwf` | Predictions |
| [benchmark.c](#benchmarking) | Performance profiling | `.bin` + `.nwf` | Timing stats |
| [create_test_weights.c](#weight-generator) | Generate random weights | - | `.nwf` |

---

## Weight Converters

### PyTorch Converter

**File:** [convert_pytorch.py](convert_pytorch.py)

**Purpose:** Convert PyTorch `.pth`/`.pt` model files to NetLang `.nwf` format.

**Usage:**
```bash
python tools/convert_pytorch.py <input.pth> <output.nwf>
```

**Example:**
```bash
# Convert a pretrained LeNet-5 model
python tools/convert_pytorch.py lenet5_trained.pth models/lenet5_mnist.nwf

# Convert VGG-16
python tools/convert_pytorch.py vgg16_pretrained.pt models/vgg16_imagenet.nwf
```

**Requirements:**
```bash
pip install torch numpy
```

**Supported PyTorch Modules:**
- `nn.Conv2d` → Conv2D weights
- `nn.Linear` → Dense weights
- `nn.BatchNorm2d` → Running mean/variance
- `nn.LayerNorm` → Scale/shift parameters

**Notes:**
- Automatically detects layer types from state_dict keys
- Converts PyTorch's (out, in, h, w) format to NetLang's internal format
- Handles both `model.state_dict()` and full checkpoint saves
- Weight alignment: 64-byte aligned for AVX2 optimization

**Output Format:**
```
[Magic: NWF0] [Version] [Num Layers] [Layer1 Metadata] [Layer1 Data aligned to 64B] ...
```

See [docs/WEIGHT_FORMAT.md](../docs/WEIGHT_FORMAT.md) for detailed specification.

---

### Keras Converter

**File:** [convert_keras.py](convert_keras.py)

**Purpose:** Convert TensorFlow/Keras `.h5` models to `.nwf` format.

**Usage:**
```bash
python tools/convert_keras.py <input.h5> <output.nwf>
```

**Example:**
```bash
# Convert a Keras MNIST model
python tools/convert_keras.py mnist_model.h5 models/mnist_cnn.nwf

# From SavedModel directory
python tools/convert_keras.py saved_model/ models/output.nwf
```

**Requirements:**
```bash
pip install tensorflow numpy
```

**Supported Keras Layers:**
- `Conv2D` → Conv2D weights + biases
- `Dense` → Dense weights + biases
- `BatchNormalization` → gamma, beta, moving_mean, moving_variance
- `LayerNormalization` → gamma, beta

**Notes:**
- Works with both Keras Sequential and Functional API models
- Automatically extracts weights in correct order
- Supports SavedModel and HDF5 formats
- Preserves layer names for debugging

---

### ONNX Converter

**File:** [convert_onnx.py](convert_onnx.py)

**Purpose:** Convert ONNX `.onnx` models to `.nwf` format.

**Usage:**
```bash
python tools/convert_onnx.py <input.onnx> <output.nwf>
```

**Example:**
```bash
# Convert an ONNX export from PyTorch
python tools/convert_onnx.py lenet5.onnx models/lenet5_mnist.nwf

# From framework-agnostic ONNX model
python tools/convert_onnx.py model.onnx models/output.nwf
```

**Requirements:**
```bash
pip install onnx numpy
```

**Supported ONNX Operators:**
- `Conv` → Conv2D
- `Gemm` / `MatMul` → Dense
- `BatchNormalization` → BatchNorm
- Initializers extracted as weights

**Notes:**
- ONNX is framework-agnostic (PyTorch, TF, Caffe, etc. → ONNX → .nwf)
- Validates model graph structure before conversion
- Extracts initializers (constant weights) from model
- Handles weight transposition between ONNX and NetLang formats

---

## Data Preprocessing

### Image Preprocessor

**File:** [preprocess.py](preprocess.py)

**Purpose:** Convert images (JPEG, PNG) to raw float32 `.bin` format for inference.

**Usage:**
```bash
python tools/preprocess.py <input.jpg> <output.bin> [options]
```

**Options:**
- `--size WIDTH HEIGHT` - Resize image (default: original size)
- `--normalize imagenet` - Apply ImageNet normalization (mean=[0.485,0.456,0.406], std=[0.229,0.224,0.225])
- `--normalize mnist` - Apply MNIST normalization (divide by 255)
- `--grayscale` - Convert to grayscale (single channel)
- `--flatten` - Flatten to 1D vector (for Dense-only networks)

**Examples:**

**LeNet-5 (MNIST):**
```bash
python tools/preprocess.py digit.png digit.bin --size 32 32 --grayscale --normalize mnist
# Output: 32×32×1 = 4096 floats = 16384 bytes
```

**VGG-16 (ImageNet):**
```bash
python tools/preprocess.py cat.jpg cat.bin --size 224 224 --normalize imagenet
# Output: 224×224×3 = 150528 floats = 602112 bytes
```

**Simple Classifier (flattened):**
```bash
python tools/preprocess.py digit.png digit.bin --size 28 28 --grayscale --flatten --normalize mnist
# Output: 784 floats = 3136 bytes
```

**Requirements:**
```bash
pip install pillow numpy
```

**Output Format:**
Raw float32 binary file (little-endian):
```
[float32] [float32] [float32] ... (H × W × C elements)
```

**Memory Layout:**
- **3D Tensors (H×W×C):** Row-major with channels last
  ```
  pixel[0,0,0], pixel[0,0,1], ..., pixel[0,0,C-1],
  pixel[0,1,0], pixel[0,1,1], ..., pixel[0,1,C-1],
  ...
  pixel[H-1,W-1,C-1]
  ```

- **1D Vectors (N):** Sequential floats
  ```
  value[0], value[1], ..., value[N-1]
  ```

---

### MNIST Downloader

**File:** [download_mnist_for_netlang.py](download_mnist_for_netlang.py)

**Purpose:** Download MNIST dataset and convert to `.bin` format (10,000 test images).

**Usage:**
```bash
python tools/download_mnist_for_netlang.py
```

**Output:**
```
test_data/preprocessed/
├── .gitkeep
├── mnist_0000_label_7.bin  (32×32×1 = 16384 bytes)
├── mnist_0001_label_2.bin
├── mnist_0002_label_1.bin
...
└── mnist_9999_label_X.bin
```

**Details:**
- Downloads MNIST test set (10,000 images) via PyTorch
- Converts from 28×28 to 32×32 with zero-padding
- Normalizes to [0, 1] range (divides by 255)
- Creates grayscale float32 binary files
- Filename includes ground truth label for validation
- Total size: ~156 MB (10,000 × 16,384 bytes)

**Requirements:**
```bash
pip install torch torchvision numpy
```

**PyTorch Cache:**
Raw MNIST data downloaded to `./data/MNIST/raw/` (~55 MB IDX format):
- `train-images-idx3-ubyte.gz` (9.9 MB)
- `train-labels-idx1-ubyte.gz` (28 KB)
- `t10k-images-idx3-ubyte.gz` (1.6 MB)
- `t10k-labels-idx1-ubyte.gz` (4.4 KB)

**Code Structure:**
```python
# Simplified version (see file for full implementation)
import torch
import torchvision
import numpy as np

# Download via PyTorch
mnist_test = torchvision.datasets.MNIST(root='./data', train=False, download=True)

# Convert each image
for idx, (image, label) in enumerate(mnist_test):
    # Convert PIL Image → numpy → 32×32 padded → normalize → .bin
    arr = np.array(image, dtype=np.float32) / 255.0  # [28, 28]
    padded = np.pad(arr, ((2,2), (2,2)), mode='constant')  # [32, 32]
    padded = padded[..., np.newaxis]  # [32, 32, 1]
    
    filename = f"test_data/preprocessed/mnist_{idx:04d}_label_{label}.bin"
    padded.astype(np.float32).tofile(filename)
```

---

## Testing & Benchmarking

### Test Harness

**File:** [test_network.c](test_network.c)

**Purpose:** C program for testing generated network inference code.

**Compilation:**
```bash
gcc -O3 -mavx2 -mfma \
    tools/test_network.c \
    generated/YOUR_NETWORK.c \
    src/codegen/runtime.c \
    src/codegen/kernels.c \
    -o bin/test_network
```

**Usage:**
```bash
./bin/test_network <input.bin> <weights.nwf>
```

**Example:**
```bash
# Test LeNet-5 on MNIST digit
./bin/test_network test_data/preprocessed/mnist_0042_label_3.bin models/lenet5_mnist.nwf
```

**Output:**
```
Loading weights from: models/lenet5_mnist.nwf
Weight file loaded successfully (348160 bytes)
Loading input from: test_data/preprocessed/mnist_0042_label_3.bin
Input loaded: 32×32×1 = 4096 floats (16384 bytes)

Running inference...
Inference time: TBD

Output: (Testing in progress)

Ground truth (from filename): 3
```

**Features:**
- Loads `.nwf` weights via mmap (zero-copy)
- Reads `.bin` input files
- Calls generated `network_infer()` function
- Measures inference time (high-resolution timer)
- Displays output probabilities (softmax)
- Validates against ground truth label (from filename)

---

### Benchmarking Tool

**File:** [benchmark.c](benchmark.c)

**Purpose:** Comprehensive performance profiling for network inference.

**Compilation:**
```bash
gcc -O3 -mavx2 -mfma \
    tools/benchmark.c \
    generated/YOUR_NETWORK.c \
    src/codegen/runtime.c \
    src/codegen/kernels.c \
    -o bin/benchmark
```

**Usage:**
```bash
./bin/benchmark <weights.nwf> <input_directory> [num_iterations]
```

**Example:**
```bash
# Benchmark LeNet-5 on 100 MNIST images (1000 iterations each)
./bin/benchmark models/lenet5_mnist.nwf test_data/preprocessed/ 1000
```

**Output:**
```
=== NetLang Benchmark Tool ===
Network: LeNet5
Weight file: models/lenet5_mnist.nwf (348 KB)
Input directory: test_data/preprocessed/
Images found: 100
Iterations per image: 1000
Total inferences: 100,000

Warming up... (10 iterations)
Running benchmark...

Progress: [####################] 100%

=== Results ===
Total time: TBD
Average inference time: TBD
Throughput: TBD

Accuracy: TBD

Layer-by-layer breakdown: (To be measured)
```

**Features:**
- Batch processing of multiple images
- Warmup phase to stabilize CPU frequency
- Statistical analysis (mean, median, min, max, std dev)
- Layer-by-layer timing breakdown
- Accuracy validation
- Throughput calculation
- Progress bar for long benchmarks

**Use Cases:**
- Comparing optimization strategies (AVX2 vs scalar)
- Profiling bottleneck layers
- Validating performance improvements
- Generating performance reports

---

### Weight Generator

**File:** [create_test_weights.c](create_test_weights.c)

**Purpose:** Generate random `.nwf` weight files for testing (when you don't have trained weights).

**Compilation:**
```bash
gcc -O2 tools/create_test_weights.c -o bin/create_weights
```

**Usage:**
```bash
./bin/create_weights <output.nwf> <architecture_spec>
```

**Example:**
```bash
# Generate weights for LeNet-5 architecture
./bin/create_weights models/lenet5_random.nwf \
    "Conv2D:6,5,5,1|Conv2D:16,5,5,6|Dense:120,400|Dense:84,120|Dense:10,84"
```

**Specification Format:**
```
"LayerType:Params|LayerType:Params|..."

Conv2D:filters,kernel_h,kernel_w,in_channels
Dense:units,input_size
```

**Notes:**
- Generates random weights with Xavier/He initialization
- 64-byte aligned for AVX2 compatibility
- Useful for compiler/runtime testing without training
- **DO NOT use for production** - accuracy will be near-random!

---

## Development Tools

### Adding New Converters

**Template for custom converter:**
```python
import numpy as np
import struct

def convert_custom_to_nwf(input_path, output_path):
    """Convert custom format to .nwf"""
    
    # 1. Load weights from your format
    weights = load_your_format(input_path)
    
    # 2. Open output file
    with open(output_path, 'wb') as f:
        # Write header
        f.write(b'NWF0')  # Magic
        f.write(struct.pack('I', 1))  # Version
        f.write(struct.pack('I', len(weights)))  # Num layers
        
        # Write each layer
        for layer in weights:
            # Metadata (layer type, dimensions)
            f.write(struct.pack('I', layer['type']))  # 0=Conv2D, 1=Dense
            f.write(struct.pack('III', *layer['shape']))
            
            # Align to 64 bytes
            pos = f.tell()
            padding = (64 - (pos % 64)) % 64
            f.write(b'\x00' * padding)
            
            # Write float32 data
            layer['data'].astype(np.float32).tofile(f)
```

See [docs/WEIGHT_FORMAT.md](../docs/WEIGHT_FORMAT.md) for complete specification.

---

## Requirements

**Python Tools:**
```bash
# Core dependencies
pip install numpy

# PyTorch converter
pip install torch torchvision

# Keras converter
pip install tensorflow

# ONNX converter
pip install onnx

# Image preprocessing
pip install pillow
```

**C Tools:**
- GCC 6.0+ or Clang 10.0+
- AVX2-capable CPU (Intel Haswell 2013+)

---

## Troubleshooting

### Conversion Issues

**"Module not found" errors:**
```bash
# Install missing dependencies
pip install torch tensorflow onnx pillow numpy
```

**"Unsupported layer type" warnings:**
- Check if your model uses unsupported layers (e.g., GRU, LSTM, Attention)
- NetLang currently supports: Conv2D, Dense, MaxPool, AvgPool, Flatten, Concat, BatchNorm
- Workaround: Export intermediate layer outputs and convert separately

**Weight shape mismatches:**
- Verify network architecture in `.nlang` matches trained model
- Check tensor format conventions (NCHW vs NHWC vs HWC)
- Use `--verbose` flag to see layer-by-layer conversion

### Preprocessing Issues

**"Image size mismatch" errors:**
- Ensure `--size` matches network input dimensions
- LeNet-5: `--size 32 32`
- VGG-16: `--size 224 224`

**Poor accuracy after preprocessing:**
- Check normalization method matches training
- ImageNet models need `--normalize imagenet`
- MNIST models need `--normalize mnist` (or divide by 255)

**Memory errors with large images:**
- Resize images before preprocessing
- Use streaming for batch processing

---

## Performance Tips

1. **Use AVX2 flags:** `-mavx2 -mfma` for 2-4× speedup
2. **Profile first:** Use `benchmark.c` to identify bottlenecks
3. **Batch preprocessing:** Convert all images at once
4. **Cache weights:** `.nwf` files are already optimized for mmap

---

## Contributing

Want to add a new tool? Follow these guidelines:
1. **Single responsibility:** One tool, one task
2. **Clear CLI:** Use argparse with helpful descriptions
3. **Error handling:** Validate inputs, provide clear error messages
4. **Documentation:** Update this README with usage examples
5. **Testing:** Provide test data and expected outputs

**Wishlist:**
- TensorFlow Lite converter
- CoreML converter (macOS)
- TensorRT converter (NVIDIA)
- INT8 quantization tool
- Model visualization tool
- .nwf inspector (dump layer info)

---

## License

MIT License - See [LICENSE](../LICENSE)

---

**See Also:**
- [Main README](../README.md) - Project overview
- [examples/README.md](../examples/README.md) - Example networks
- [docs/WEIGHT_FORMAT.md](../docs/WEIGHT_FORMAT.md) - .nwf specification
