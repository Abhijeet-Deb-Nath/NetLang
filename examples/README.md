# NetLang Example Networks

Example architectures demonstrating NetLang capabilities.

---

## Available Examples

| Example | Description | Input Size | Parameters |
|---------|-------------|------------|-----------|
| **simple_classifier.nlang** | Dense-only network | 784×1 | ~100K |
| **lenet5.nlang** | LeNet-5 (MNIST) | 32×32×1 | 60K |
| **lenet_trained.nlang** | LeNet-5 with trained weights | 28×28×1 | 60K |
| **vgg16.nlang** | VGG-16 (ImageNet) | 224×224×3 | 138M |

---

## Usage

**1. Compile example:**
```bash
bin/netlang_compiler examples/lenet5.nlang -o generated/lenet5.c
```

**2. Build executable:**
```bash
gcc -O3 -mavx2 generated/lenet5.c -o lenet5_infer -lm
```

**3. Run inference:**
```bash
./lenet5_infer test_data/mnist_test_0.bin
```

---

## Architecture Details

**Simple Classifier**
```
Input (784) → Dense(128, relu) → Dense(64, relu) → Dense(10, softmax)
```
Minimal example for learning NetLang syntax.

**LeNet-5**
```
Input (32×32×1)
  → Conv2D(6, 5×5, relu) → MaxPool(2×2)
  → Conv2D(16, 5×5, relu) → MaxPool(2×2)
  → Flatten() → Dense(120, relu) → Dense(84, relu) → Dense(10, softmax)
```
Classic CNN for digit recognition (MNIST).

**VGG-16**
```
13 Conv2D layers + 3 Dense layers
224×224×3 input, ~138M parameters
```
Deep CNN for ImageNet classification.

---

## Creating Custom Networks

See [docs/USER_GUIDE.md](../docs/USER_GUIDE.md) for syntax reference and layer documentation.
