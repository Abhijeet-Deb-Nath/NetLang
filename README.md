# NetLang Compiler

**DSL compiler transforming neural network architectures into optimized C code with AVX2 vectorization.**

---

## Quick Example

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
    output = Dense(units: 10, activation: softmax) from x
}
```
**Output:** Optimized C code → Standalone executable

---

## Features

- Ahead-of-time compilation with shape specialization
- AVX2 vectorization for Conv2D/Dense/Pool layers
- Automatic shape inference and type checking
- Memory-mapped weight loading (~1ms)
- Import weights from PyTorch/ONNX/Keras
- Zero runtime dependencies

---

## Quick Start

```bash
# 1. Build compiler
build.bat

# 2. Compile network to C
bin/netlang_compiler architecture/lenet5.nlang -o generated/lenet5.c

# 3. Build executable
gcc -O3 -mavx2 generated/lenet5.c -o lenet5_infer -lm

# 4. Run inference
./lenet5_infer input.bin
```

**See [QUICKSTART.md](QUICKSTART.md) for detailed 5-minute guide.**

---

## Performance

**LeNet5 on MNIST (Intel i5-5200U):**
- Inference: 4.43ms per image (with optimizations)
- Throughput: 0.98 GFLOPS
- Accuracy: 100% on 100 test images

**Optimizations applied:** Operator fusion, L1 cache blocking  
**Gap vs ONNX Runtime:** 15× (expected for research compiler)

**See [BENCHMARK_RESULTS.md](BENCHMARK_RESULTS.md) for detailed analysis.**

---

## Documentation

- **[QUICKSTART.md](QUICKSTART.md)** - Quick start guide
- **[BENCHMARK_RESULTS.md](BENCHMARK_RESULTS.md)** - Performance analysis
- **[docs/USER_GUIDE.md](docs/USER_GUIDE.md)** - Complete syntax reference
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - Compiler internals
- **[docs/OPTIMIZATION_RESULTS.md](docs/OPTIMIZATION_RESULTS.md)** - Detailed optimization results

---

## Project Structure

```
flex_work/
├── src/                   # Compiler (Lex/Bison/C)
│   ├── lexer/            # Tokenization
│   ├── parser/           # Syntax analysis
│   ├── ast/              # Abstract Syntax Tree
│   ├── semantic/         # Type checking & shape inference
│   └── codegen/          # Code generation + optimizations
│
├── architecture/          # Network definitions (.nlang)
├── models/                # Pretrained weights (.nwf)
├── tools/                 # PyTorch/ONNX converters
├── docs/                  # Documentation
├── generated/             # Generated C code (output)
└── bin/                   # Compiled executables (output)
```

---

## Supported Layers

Conv2D, Dense, MaxPool, AvgPool, Flatten, BatchNorm, ReLU, Softmax

---

## Requirements

- GCC/Clang with AVX2 support
- Flex 2.6+, Bison 3.0+
- x86-64 CPU with AVX2 (Intel Haswell 2013+)
- Python 3.7+ (optional, for weight conversion)

---

## License

MIT License
