# NetLang Quick Start

Get from zero to running inference in **5 minutes**.

---

## Prerequisites

**Windows:**
- [MSYS2](https://www.msys2.org/) with GCC, Flex, Bison
- Python 3.7+ (optional, for weight conversion)

**Linux/macOS:**
- GCC/Clang, flex, bison, make
- Python 3.7+ (optional)

**Verify:**
```bash
gcc --version
flex --version
bison --version
```

---

## Step 1: Build the Compiler (30 seconds)

```bash
# Windows
build.bat

# Linux/macOS
make
```

**Output:** `build/netlang.exe` (or `build/netlang` on Unix)

---

## Step 2: Compile a Network (5 seconds)

```bash
# Compile LeNet-5 architecture to C code
bin/netlang_compiler architecture/lenet5.nlang -o generated/lenet5.c

# Or on Windows:
build\netlang.exe architecture\lenet5.nlang -o generated\lenet5.c
```

**Output:** `generated/lenet5.c` (optimized C code with AVX2)

---

## Step 3: Build Executable (10 seconds)

```bash
gcc -O3 -mavx2 generated/lenet5.c -o lenet5_infer -lm
```

**Output:** `lenet5_infer` executable (or `lenet5_infer.exe`)

---

## Step 4: Run Inference

### Option A: Test with Synthetic Data

```bash
# Create a simple test input (28x28 zeros)
python -c "import numpy as np; np.zeros((28,28), dtype='float32').tofile('test_input.bin')"

# Run inference
./lenet5_infer test_input.bin
```

### Option B: Test with Real MNIST Data

```bash
# Download and preprocess MNIST
python tools/download_mnist_for_netlang.py

# Run on real test image
./lenet5_infer test_data/mnist_test_0.bin
```

**Expected output:**
```
Class probabilities:
0: 0.1234
1: 0.0567
...
9: 0.0823

Predicted: 7 (confidence: 95.3%)
```

---

## What Just Happened?

1. **NetLang compiler** parsed `.nlang` → generated optimized C code
2. **GCC** compiled C code → created standalone executable
3. **Executable** loaded weights via mmap → ran inference with AVX2

**Performance:**
- Compilation: ~50ms
- Weight loading: ~1ms  
- Inference: ~10-13ms per image

---

## Next Steps

### Run Accuracy Test (500 images)

```bash
python test_accuracy.py \
    --network architecture/lenet5.nlang \
    --weights models/lenet5_mnist.nwf \
    --num-samples 500
```

**Expected:** ~97% accuracy

### Try a Different Architecture

```bash
# VGG-16 (larger network)
bin/netlang_compiler architecture/vgg16.nlang -o generated/vgg16.c
gcc -O3 -mavx2 generated/vgg16.c -o vgg16_infer -lm
```

### Use Your Own Model

```bash
# Convert PyTorch model
python tools/convert_pytorch.py your_model.pth models/your_model.nwf

# Create .nlang architecture file (see examples/)
# Compile and run
```

---

## Common Issues

**"netlang: command not found"**
→ Use `build/netlang.exe` or `./build/netlang`

**"undefined reference to `__mm256_loadu_ps`"**
→ Add `-mavx2` flag to gcc

**"Cannot open weight file"**
→ Ensure `.nwf` path in `.nlang` is correct and relative to execution directory

---

## Documentation

- **[USER_GUIDE.md](docs/USER_GUIDE.md)** - Complete workflows and syntax
- **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** - How the compiler works
- **[OPTIMIZATION_ROADMAP.md](docs/OPTIMIZATION_ROADMAP.md)** - Performance improvements

---

## Example: Complete MNIST Pipeline

```bash
# 1. Build compiler
build.bat

# 2. Download MNIST data
python tools/download_mnist_for_netlang.py

# 3. Compile network
build\netlang.exe architecture\lenet5.nlang -o generated\lenet5.c

# 4. Build executable
gcc -O3 -mavx2 generated\lenet5.c -o lenet5_infer.exe -lm

# 5. Test accuracy
python test_accuracy.py --network architecture\lenet5.nlang --num-samples 100

# Output: ~97% accuracy, ~12ms per image
```

---

**You're ready!** See [USER_GUIDE.md](docs/USER_GUIDE.md) for detailed documentation.
