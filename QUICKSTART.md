# NetLang Quick Start

Get from zero to inference in **5 minutes**.

---

## Prerequisites

**Windows:** MSYS2 with GCC, Flex, Bison  
**Linux/macOS:** GCC/Clang, flex, bison, make  
**Optional:** Python 3.7+ (for weight conversion)

---

## Step 1: Build Compiler

```bash
# Windows
build.bat

# Linux/macOS
make
```

---

## Step 2: Compile Network to C

```bash
# Windows
build\netlang.exe architecture\lenet5.nlang -o generated\lenet5.c

# Linux/macOS
bin/netlang_compiler architecture/lenet5.nlang -o generated/lenet5.c
```

---

## Step 3: Build Executable

```bash
gcc -O3 -mavx2 generated/lenet5.c -o lenet5_infer -lm
```

---

## Step 4: Run Inference

**Option A: Synthetic test data**
```bash
python -c "import numpy as np; np.zeros((28,28), dtype='float32').tofile('test.bin')"
./lenet5_infer test.bin
```

**Option B: Real MNIST data**
```bash
python tools/download_mnist_for_netlang.py
./lenet5_infer test_data/mnist_test_0.bin
```

**Output:**
```
Predicted: 7 (confidence: 95.3%)
Inference time: 4.43 ms
```

---

## Next Steps

**Run accuracy test:**
```bash
python test_accuracy.py --network architecture/lenet5.nlang --num-samples 100
```

**Try VGG-16:**
```bash
bin/netlang_compiler architecture/vgg16.nlang -o generated/vgg16.c
gcc -O3 -mavx2 generated/vgg16.c -o vgg16_infer -lm
```

**Convert your own model:**
```bash
python tools/convert_pytorch.py your_model.pth models/your_model.nwf
# Then create .nlang file (see examples/)
```

---

## Documentation

- **[README.md](README.md)** - Project overview
- **[BENCHMARK_RESULTS.md](BENCHMARK_RESULTS.md)** - Performance results
- **[docs/USER_GUIDE.md](docs/USER_GUIDE.md)** - Complete syntax reference
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - Compiler internals
