# NetLang Compiler

**A domain-specific language compiler for neural network inference on x86-64 CPUs.**

NetLang transforms high-level CNN architectures into optimized C code with automatic shape inference, type checking, and AVX2 vectorization. Designed for fast single-image CPU inference with minimal dependencies.

---

## Quick Example

**Input (NetLang DSL):**
```netlang
network LeNet5 {
    input(shape: [28, 28, 1])
    weights("models/lenet5_mnist.nwf")
    
    x = Conv2D(filters: 6, kernel: [5, 5], activation: relu) from input
    x = MaxPool(pool: [2, 2], stride: 2) from x
    x = Conv2D(filters: 16, kernel: [5, 5], activation: relu) from x
    x = MaxPool(pool: [2, 2], stride: 2) from x
    x = Flatten() from x
    x = Dense(units: 120, activation: relu) from x
    x = Dense(units: 84, activation: relu) from x
    output = Dense(units: 10, activation: softmax) from x
}
```

**Output:** Single-file C code with AVX2 SIMD → Compile to standalone executable

---

## Features

- ⚡ **Ahead-of-Time Compilation** - No JIT overhead, predictable performance
- 🎯 **Shape Specialization** - Optimized code for specific input dimensions
- 🚀 **AVX2 Vectorization** - 8-wide SIMD for Conv2D/Dense/Pool layers
- 🔍 **Automatic Shape Inference** - No manual tensor calculations
- 🛡️ **Strong Type Checking** - Catch shape mismatches at compile time
- 📦 **Fast Weight Loading** - Memory-mapped .nwf format (~1ms startup)
- 🔧 **Framework Interop** - Import weights from PyTorch, ONNX, Keras
- 📄 **Zero Dependencies** - Self-contained executable, no runtime libraries

---

## Quick Start

See [QUICKSTART.md](QUICKSTART.md) for a 5-minute guide from zero to inference.

**TL;DR:**

```bash
# 1. Build compiler
build.bat

# 2. Compile network
bin/netlang_compiler architecture/lenet5.nlang -o generated/lenet5.c

# 3. Build executable
gcc -O3 -mavx2 generated/lenet5.c -o lenet5_infer -lm

# 4. Run
./lenet5_infer input.bin
```

---

## Performance

**Current (LeNet5 on MNIST):**
- Compilation: ~50ms (.nlang → .c)
- Inference: ~10-13ms per image (28×28×1)
- Accuracy: 97% on MNIST test set

**Planned Optimizations:** 10-15x speedup through operator fusion, cache blocking, and multi-threading. See [OPTIMIZATION_ROADMAP.md](docs/OPTIMIZATION_ROADMAP.md).

---

## Documentation

**Getting Started:**
- **[QUICKSTART.md](QUICKSTART.md)** - 5-minute quick start guide
- **[User Guide](docs/USER_GUIDE.md)** - Complete workflows and syntax reference

**Understanding the System:**
- **[Architecture](docs/ARCHITECTURE.md)** - Compiler internals and design
- **[Weight Format](docs/WEIGHT_FORMAT.md)** - .nwf binary format specification

**Performance & Development:**
- **[Optimization Roadmap](docs/OPTIMIZATION_ROADMAP.md)** - Performance improvement plan
- **[Implementation Guide](docs/IMPLEMENTATION_GUIDE.md)** - Extending the compiler
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Contribution guidelines

---

## Project Structure

```
flex_work/
├── src/                   # Compiler source code (C)
│   ├── lexer/            # Tokenization (Flex)
│   ├── parser/           # Syntax analysis (Bison)
│   ├── ast/              # Abstract Syntax Tree
│   ├── semantic/         # Type checking & shape inference
│   └── codegen/          # C code generation with AVX2 kernels
│
├── architecture/          # Network definitions (.nlang files)
│   ├── lenet5.nlang      # LeNet-5 architecture
│   └── vgg16.nlang       # VGG-16 architecture
│
├── models/                # Pretrained weights (.nwf format)
│   └── lenet5_mnist.nwf  # LeNet-5 trained on MNIST
│
├── tools/                 # Utilities
│   ├── convert_pytorch.py         # Convert PyTorch models
│   ├── convert_onnx.py            # Convert ONNX models
│   ├── preprocess.py              # Image preprocessing
│   └── test_network.c             # C test harness
│
├── docs/                  # Documentation
│   ├── USER_GUIDE.md              # Complete usage guide
│   ├── ARCHITECTURE.md            # Compiler internals
│   ├── WEIGHT_FORMAT.md           # .nwf specification
│   └── OPTIMIZATION_ROADMAP.md    # Performance plan
│
├── generated/             # Generated C code (output)
├── bin/                   # Compiled executables (output)
└── test_data/             # Test images
```

---

## Supported Layers

| Layer | Example | Notes |
|-------|---------|-------|
| Conv2D | `Conv2D(filters: 64, kernel: [3,3], activation: relu)` | AVX2 optimized |
| Dense | `Dense(units: 128, activation: relu)` | Fully connected |
| MaxPool | `MaxPool(pool: [2,2], stride: 2)` | Max pooling |
| AvgPool | `AvgPool(pool: [2,2])` | Average pooling |
| Flatten | `Flatten()` | Reshape to 1D |
| BatchNorm | `BatchNorm()` | Batch normalization |

---

## Example: MNIST Classification

**1. Write architecture definition (already provided):**

[architecture/lenet5.nlang](architecture/lenet5.nlang):
```netlang
network LeNet5 {
    input(shape: [28, 28, 1])
    weights("models/lenet5_mnist.nwf")
    
    x = Conv2D(filters: 6, kernel: [5, 5], activation: relu) from input
    x = MaxPool(pool: [2, 2], stride: 2) from x
    x = Conv2D(filters: 16, kernel: [5, 5], activation: relu) from x
    x = MaxPool(pool: [2, 2], stride: 2) from x
    x = Flatten() from x
    x = Dense(units: 120, activation: relu) from x
    x = Dense(units: 84, activation: relu) from x
    output = Dense(units: 10, activation: softmax) from x
}
```

**2. Test the system:**

See [USER_GUIDE.md](docs/USER_GUIDE.md) for complete examples and workflow.

---

## Requirements

**Build Requirements:**
- GCC/Clang with AVX2 support
- Flex 2.6+, Bison 3.0+
- x86-64 CPU with AVX2 (Intel Haswell 2013+)

**Optional (for tools):**
- Python 3.7+
- PyTorch / ONNX (for weight conversion)

---

## Current Status

**Working:**
- ✅ Full compilation pipeline (97% accuracy on MNIST)
- ✅ LeNet5, VGG16, custom architectures
- ✅ Weight conversion from PyTorch/ONNX
- ✅ AVX2 vectorization
- ✅ ~10-13ms inference time per MNIST image

**Planned Optimizations:**
- Operator fusion (Conv+ReLU): 2.5x speedup
- Cache blocking: 3x speedup
- Multi-threading: 2x speedup
- FMA instructions: 1.3x speedup
- **Target: 0.6-1ms per image (15x improvement)**

See [OPTIMIZATION_ROADMAP.md](docs/OPTIMIZATION_ROADMAP.md) for details.

---

## Contributing

Contributions welcome! See documentation for architecture details and extension points.

---

## License

MIT License

---

*NetLang: Compile once, infer fast.*
