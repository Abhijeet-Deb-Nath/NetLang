# NetLang Compiler

**A domain-specific language for compiling neural network architectures into optimized x86-64 machine code**

NetLang compiles high-level CNN definitions into efficient C code with automatic shape inference, type checking, and AVX2 vectorization for fast CPU inference.

---

## Quick Example

**Input (NetLang):**
```netlang
network LeNet5 {
    input(shape: [32, 32, 1])
    weights("models/lenet5_mnist.nwf")
    
    x = Conv2D(filters: 6, kernel: [5, 5], activation: relu) from input
    x = MaxPool(pool: [2, 2], stride: 2) from x
    x = Conv2D(filters: 16, kernel: [5, 5], activation: relu) from x
    x = MaxPool(pool: [2, 2], stride: 2) from x
    x = Flatten() from x
    x = Dense(units: 120, activation: relu) from x
    x = Dense(units: 84, activation: relu) from x
    x = Dense(units: 10, activation: softmax) from x
}
```

**Output:** Optimized C code with AVX2 intrinsics

---

## Features

- ⚡ **Fast Compilation** - Compiles .nlang to C in milliseconds
- 🎯 **Static Optimization** - Shape specialization, operator fusion
- 🚀 **AVX2 Vectorization** - 8-wide SIMD operations
- 🔍 **Automatic Shape Inference** - No manual tensor size calculations
- 🛡️ **Type Safety** - Compile-time shape mismatch detection
- 📦 **Zero-Copy Weight Loading** - Custom .nwf format with mmap (~1ms)
- 🔧 **Framework Interop** - Convert from PyTorch, Keras, ONNX

---

## Project Structure

```
netlang/
├── src/                   # Compiler source code (C)
│   ├── lexer/            # Tokenization (Flex)
│   ├── parser/           # Syntax analysis (Bison)
│   ├── ast/              # Abstract Syntax Tree
│   ├── semantic/         # Type checking & shape inference
│   └── codegen/          # C code generation & runtime kernels
│
├── examples/              # Example networks
│   ├── lenet5.nlang      # LeNet-5 for MNIST (32×32, 10 classes)
│   ├── vgg16.nlang       # VGG-16 for ImageNet (224×224, 1000 classes)
│   └── simple_classifier.nlang  # Basic dense network
│
├── models/                # Pretrained weights (.nwf format)
│   ├── lenet5_mnist.nwf  # LeNet-5 trained on MNIST (~348 KB)
│   └── mnist_cnn.nwf     # Legacy variant
│
├── tools/                 # Utilities
│   ├── convert_pytorch.py     # PyTorch → .nwf converter
│   ├── convert_keras.py       # Keras → .nwf converter
│   ├── convert_onnx.py        # ONNX → .nwf converter
│   ├── preprocess.py          # Image → .bin preprocessor
│   ├── download_mnist_for_netlang.py  # MNIST dataset downloader
│   ├── test_network.c         # C test harness for inference
│   └── benchmark.c            # Performance benchmarking
│
├── build/                 # Compiler build artifacts (gitignored)
│   └── netlang.exe       # The NetLang compiler
│
├── generated/             # Generated C networks (gitignored)
├── bin/                   # Compiled network executables (gitignored)
├── data/                  # Downloaded datasets (gitignored)
└── test_data/             # Test images (.bin format, gitignored)
```

---

## Quick Start

### 1. Build the Compiler

**Linux/macOS:**
```bash
make
```

**Windows:**
```batch
build.bat
```

### 2. Compile a Network

```bash
# Compile LeNet-5 architecture to C code
./build/netlang examples/lenet5.nlang -o generated/lenet5.c

# Build the executable with AVX2 optimizations
gcc -O3 -march=haswell -mavx2 -mfma \
    generated/lenet5.c \
    src/codegen/runtime.c \
    src/codegen/kernels.c \
    -o bin/lenet5_infer
```

### 3. Prepare Data

```bash
# Download and preprocess MNIST test dataset (10,000 images)
python tools/download_mnist_for_netlang.py
# Output: test_data/preprocessed/mnist_0000_label_7.bin ... mnist_9999_label_X.bin
```

### 4. Run Inference

```bash
# Run on a sample MNIST digit
./bin/lenet5_infer test_data/preprocessed/mnist_0000_label_7.bin

# Output: (Testing in progress)
```

---

## Key Concepts

### Architecture vs Dataset

- **Architecture** = Network structure (e.g., LeNet-5, VGG-16, ResNet-50)
- **Dataset** = Training data (e.g., MNIST, ImageNet, CIFAR-10)
- **Model** = Architecture + Trained Weights

**Example:**
- `lenet5.nlang` defines the **LeNet-5 architecture**
- `lenet5_mnist.nwf` contains **weights trained on MNIST**
- Together they form a **MNIST digit classifier model**

### File Formats

| Extension | Purpose | Example |
|-----------|---------|---------|
| `.nlang` | Network architecture definition | `examples/lenet5.nlang` |
| `.nwf` | Binary weight file (NetLang Weight Format) | `models/lenet5_mnist.nwf` |
| `.bin` | Preprocessed input image (raw float32) | `test_data/mnist_0000_label_7.bin` |
| `.c` | Generated C inference code | `generated/lenet5.c` |

### Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│ COMPILATION PHASE (One-time)                                │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  lenet5.nlang ──┬──> NetLang Compiler ──> lenet5.c         │
│                 │         (netlang.exe)                      │
│  lenet5_mnist.nwf ──> (Referenced in .nlang)                │
│                                                              │
│  lenet5.c + runtime.c + kernels.c ──> GCC ──> lenet5_infer │
│                                                              │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ INFERENCE PHASE (Every prediction)                          │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  image.jpg ──> preprocess.py ──> image.bin                  │
│                                                              │
│  image.bin + lenet5_mnist.nwf ──> lenet5_infer ──> Result  │
│                (loaded via mmap)                             │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## Supported Layers

| Layer | Syntax | Description |
|-------|--------|-------------|
| **Conv2D** | `Conv2D(filters: 64, kernel: [3,3], padding: 1, activation: relu)` | 2D Convolution with AVX2 optimization |
| **Dense** | `Dense(units: 128, activation: relu)` | Fully connected layer |
| **MaxPool** | `MaxPool(pool: [2,2], stride: 2)` | Max pooling |
| **AvgPool** | `AvgPool(pool: [2,2], stride: 2)` | Average pooling |
| **Flatten** | `Flatten()` | Reshape 3D → 1D |
| **Concat** | `Concat(axis: 2)` | Concatenate tensors |

**Activations:** `relu`, `softmax`

---

## Performance

**Test Machine:** Intel Core i5-5200U @ 2.2GHz (2015 laptop CPU)

| Architecture | Dataset | Input Size | Inference Time | Throughput |
|-------------|---------|------------|----------------|------------|
| LeNet-5 | MNIST | 32×32×1 | TBD | TBD |
| VGG-16 | ImageNet | 224×224×3 | TBD | TBD |

**Optimizations:**
- AVX2 SIMD (8 floats per instruction)
- FMA (fused multiply-add)
- Zero-copy weight loading via mmap
- 64-byte aligned memory for cache efficiency

*Performance benchmarks will be updated after testing is complete.*

---

## Documentation

- **[QUICKSTART.md](QUICKSTART.md)** - Detailed getting started guide
- **[examples/README.md](examples/README.md)** - Example networks guide
- **[docs/WEIGHT_FORMAT.md](docs/WEIGHT_FORMAT.md)** - .nwf format specification
- **[docs/IMPLEMENTATION_GUIDE.md](docs/IMPLEMENTATION_GUIDE.md)** - Compiler internals
- **[PERFORMANCE_SYSTEM_COMPLETE.md](PERFORMANCE_SYSTEM_COMPLETE.md)** - Benchmarking details

---

## Examples

### LeNet-5 for MNIST
```bash
# 1. Compile network
./build/netlang examples/lenet5.nlang -o generated/lenet5.c

# 2. Build executable
gcc -O3 -mavx2 -mfma generated/lenet5.c src/codegen/runtime.c src/codegen/kernels.c -o bin/lenet5

# 3. Download MNIST dataset
python tools/download_mnist_for_netlang.py

# 4. Run inference
./bin/lenet5 test_data/preprocessed/mnist_0042_label_3.bin
```

### VGG-16 for ImageNet
```bash
# 1. Convert pretrained PyTorch model
python tools/convert_pytorch.py vgg16.pth models/vgg16_imagenet.nwf

# 2. Compile network
./build/netlang examples/vgg16.nlang -o generated/vgg16.c

# 3. Build executable
gcc -O3 -mavx2 generated/vgg16.c src/codegen/runtime.c src/codegen/kernels.c -o bin/vgg16

# 4. Preprocess image
python tools/preprocess.py cat.jpg cat.bin --size 224 224

# 5. Run inference
./bin/vgg16 cat.bin
```

See [examples/README.md](examples/README.md) for more examples.

---

## Development Status

| Component | Status |
|-----------|--------|
| Lexical Analysis | ✅ Complete |
| Parser & AST | ✅ Complete |
| Semantic Analysis | ✅ Complete |
| Shape Inference | ✅ Complete |
| Type Checking | ✅ Complete |
| Weight Format (.nwf) | ✅ Complete |
| Runtime Library | ✅ Complete |
| C Code Generator | ✅ Complete |
| AVX2 Kernels | ✅ Complete |
| Weight Converters | ✅ Complete |
| Preprocessing Tools | ✅ Complete |
| Test Infrastructure | ✅ Complete |

---

## Converting Pretrained Models

### From PyTorch
```bash
python tools/convert_pytorch.py model.pth models/output.nwf
```

### From Keras/TensorFlow
```bash
python tools/convert_keras.py model.h5 models/output.nwf
```

### From ONNX
```bash
python tools/convert_onnx.py model.onnx models/output.nwf
```

---

## Requirements

### Compiler Build
- **C Compiler:** GCC 6.0+ or Clang 10.0+
- **Build Tools:** flex, bison, make
- **CPU:** x86-64 with AVX2 support (Intel Haswell 2013+, AMD Excavator 2015+)

### Runtime (Generated Code)
- **C Compiler:** Any C99-compliant compiler with AVX2 intrinsics
- **OS:** Linux, macOS, Windows (MinGW)

### Tools (Optional)
- **Python 3.7+** for weight conversion and preprocessing
- **PyTorch / TensorFlow / ONNX** (only for conversion)

---

## Contributing

Contributions welcome! Areas of interest:
- Additional layer types (Batch/LayerNorm, Residual blocks)
- ARM NEON backend for mobile/embedded
- LLVM IR backend for multi-platform support
- INT8/FP16 quantization for faster inference
- Training support (backpropagation)

---

## License

MIT License - See [LICENSE](LICENSE) file

---

## References

- **LeNet-5:** LeCun et al., "Gradient-Based Learning Applied to Document Recognition" (1998)
- **VGG:** Simonyan & Zisserman, "Very Deep Convolutional Networks for Large-Scale Image Recognition" (2014)
- **MNIST:** http://yann.lecun.com/exdb/mnist/

---

## Citation

If you use NetLang in your research, please cite:
```bibtex
@software{netlang2026,
  title={NetLang: A Domain-Specific Language for CNN Compilation},
  author={Your Name},
  year={2026},
  url={https://github.com/yourusername/netlang}
}
```

---

**Target:** CPU inference at the edge | **Focus:** Compile-time optimization | **Philosophy:** Static is faster

*"Compile once, infer fast."*
