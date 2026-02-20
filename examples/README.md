# NetLang Example Networks

This directory contains example network architectures demonstrating NetLang's capabilities, from simple classifiers to complex CNN architectures.

---

## Quick Reference

| Example | Architecture | Dataset | Input Size | Parameters |
|---------|-------------|---------|------------|-----------|
| [simple_classifier.nlang](#simple-classifier) | Dense-only | Generic | 784×1 | ~100K |
| [lenet5.nlang](#lenet-5-mnist) | LeNet-5 | MNIST | 32×32×1 | ~60K |
| [vgg16.nlang](#vgg-16-imagenet) | VGG-16 | ImageNet | 224×224×3 | ~138M |

*Inference benchmarks to be measured after testing is complete*

---

## Simple Classifier

**File:** [simple_classifier.nlang](simple_classifier.nlang)

**Purpose:** Minimal example for beginners - a basic feedforward network with no convolutions.

**Architecture:**
```
Input (784) → Dense(128, relu) → Dense(64, relu) → Dense(10, softmax)
```

**Use Cases:**
- Learning NetLang syntax
- Testing compiler pipeline
- Simple classification tasks (MNIST flattened)

**Compile & Run:**
```bash
# 1. Compile network
./build/netlang examples/simple_classifier.nlang -o generated/simple.c

# 2. Build executable
gcc -O3 -mavx2 generated/simple.c src/codegen/runtime.c src/codegen/kernels.c -o bin/simple

# 3. Prepare input (flattened 28×28 image)
python tools/preprocess.py digit.jpg digit.bin --flatten

# 4. Run inference
./bin/simple digit.bin
```

**Key Concepts:**
- No spatial structure (1D vectors only)
- Single `Flatten()` operation before Dense layers
- Smallest memory footprint (~400KB weights)

---

## LeNet-5 (MNIST)

**File:** [lenet5.nlang](lenet5.nlang)

**Purpose:** Classic CNN architecture for handwritten digit recognition.

**Architecture:**
```
Input (32×32×1)
  → Conv2D(6, 5×5, relu) → [28×28×6]
  → MaxPool(2×2) → [14×14×6]
  → Conv2D(16, 5×5, relu) → [10×10×16]
  → MaxPool(2×2) → [5×5×16]
  → Flatten() → [400]
  → Dense(120, relu) → [120]
  → Dense(84, relu) → [84]
  → Dense(10, softmax) → [10]
```

**Details:**
- **Paper:** LeCun et al., "Gradient-Based Learning Applied to Document Recognition" (1998)
- **Dataset:** MNIST (60,000 training, 10,000 test images)
- **Input:** 32×32×1 grayscale (zero-padded from 28×28)
- **Parameters:** ~60,000
- **Weight File:** `models/lenet5_mnist.nwf` (~348 KB)
- **Inference Time:** TBD (testing in progress)

**Compile & Run:**
```bash
# 1. Download MNIST dataset
python tools/download_mnist_for_netlang.py
# Output: test_data/preprocessed/mnist_0000_label_7.bin ... mnist_9999_label_X.bin

# 2. Compile network
./build/netlang examples/lenet5.nlang -o generated/lenet5.c

# 3. Build executable
gcc -O3 -mavx2 -mfma generated/lenet5.c src/codegen/runtime.c src/codegen/kernels.c -o bin/lenet5

# 4. Run inference
./bin/lenet5 test_data/preprocessed/mnist_0042_label_3.bin
# Output: (Testing in progress)
```

**Key Concepts:**
- First successful CNN architecture (1998)
- Demonstrates spatial feature learning with convolutions
- Standard benchmark for compiler validation
- Zero-padding from 28×28 to 32×32 simplifies pooling math

**Accuracy:**
- MNIST Test Set: TBD (testing in progress)

---

## VGG-16 (ImageNet)

**File:** [vgg16.nlang](vgg16.nlang)

**Purpose:** Deep CNN for large-scale image classification (1000 classes).

**Architecture:**
```
Input (224×224×3)
  
  # Block 1
  → Conv2D(64, 3×3, padding: 1, relu) → [224×224×64]
  → Conv2D(64, 3×3, padding: 1, relu) → [224×224×64]
  → MaxPool(2×2) → [112×112×64]
  
  # Block 2
  → Conv2D(128, 3×3, padding: 1, relu) → [112×112×128]
  → Conv2D(128, 3×3, padding: 1, relu) → [112×112×128]
  → MaxPool(2×2) → [56×56×128]
  
  # Block 3
  → Conv2D(256, 3×3, padding: 1, relu) → [56×56×256]
  → Conv2D(256, 3×3, padding: 1, relu) → [56×56×256]
  → Conv2D(256, 3×3, padding: 1, relu) → [56×56×256]
  → MaxPool(2×2) → [28×28×256]
  
  # Block 4
  → Conv2D(512, 3×3, padding: 1, relu) → [28×28×512]
  → Conv2D(512, 3×3, padding: 1, relu) → [28×28×512]
  → Conv2D(512, 3×3, padding: 1, relu) → [28×28×512]
  → MaxPool(2×2) → [14×14×512]
  
  # Block 5
  → Conv2D(512, 3×3, padding: 1, relu) → [14×14×512]
  → Conv2D(512, 3×3, padding: 1, relu) → [14×14×512]
  → Conv2D(512, 3×3, padding: 1, relu) → [14×14×512]
  → MaxPool(2×2) → [7×7×512]
  
  # Classifier
  → Flatten() → [25088]
  → Dense(4096, relu) → [4096]
  → Dense(4096, relu) → [4096]
  → Dense(1000, softmax) → [1000]
```

**Details:**
- **Paper:** Simonyan & Zisserman, "Very Deep Convolutional Networks for Large-Scale Image Recognition" (2014)
- **Dataset:** ImageNet ILSVRC-2012 (1.2M training, 50K validation, 1000 classes)
- **Input:** 224×224×3 RGB
- **Parameters:** ~138 million
- **Weight File:** `models/vgg16_imagenet.nwf` (~528 MB)
- **Inference Time:** TBD (testing in progress)

**Compile & Run:**
```bash
# 1. Convert pretrained weights (if you have a PyTorch model)
python tools/convert_pytorch.py vgg16_pretrained.pth models/vgg16_imagenet.nwf

# 2. Compile network
./build/netlang examples/vgg16.nlang -o generated/vgg16.c

# 3. Build executable
gcc -O3 -mavx2 -mfma generated/vgg16.c src/codegen/runtime.c src/codegen/kernels.c -o bin/vgg16

# 4. Preprocess image
python tools/preprocess.py cat.jpg cat.bin --size 224 224 --normalize imagenet

# 5. Run inference
./bin/vgg16 cat.bin
# Output: (Testing in progress)
```

**Key Concepts:**
- Demonstrates depth scaling (16 weight layers)
- Uniform 3×3 convolutions with padding=1 preserve spatial dimensions
- 5 convolutional blocks followed by 3 dense layers
- Very large model (~528 MB weights)
- Standard ImageNet baseline (Top-5 accuracy ~90%)

**Performance Notes:**
- CPU inference is slow due to model size
- AVX2 optimization provides ~20% speedup over naive implementation
- GPU version (if ported) would be ~20× faster

---

## Other Examples

### Inception Modules
**File:** [inception.nlang](inception.nlang)

**Purpose:** Demonstrates module system with parallel branches.

**Architecture Pattern:**
```netlang
module InceptionBlock {
    input x
    
    branch1 = Conv2D(filters: 64, kernel: [1,1], activation: relu) from x
    
    branch2 = Conv2D(filters: 96, kernel: [1,1], activation: relu) from x
    branch2 = Conv2D(filters: 128, kernel: [3,3], padding: 1, activation: relu) from branch2
    
    branch3 = Conv2D(filters: 16, kernel: [1,1], activation: relu) from x
    branch3 = Conv2D(filters: 32, kernel: [5,5], padding: 2, activation: relu) from branch3
    
    branch4 = MaxPool(pool: [3,3], stride: 1, padding: 1) from x
    branch4 = Conv2D(filters: 32, kernel: [1,1], activation: relu) from branch4
    
    output = Concat(axis: 2) from [branch1, branch2, branch3, branch4]  # [H, W, 256]
}
```

**Key Concepts:**
- Multi-scale feature extraction via parallel branches
- 1×1 convolutions for dimensionality reduction
- Concatenation along channel axis
- Used in GoogLeNet/Inception architecture (2014)

### Test Networks
Testing compiler edge cases:

- **test_minimal_valid.nlang** - Minimal valid network (2 layers)
- **test_empty.nlang** - Empty network (should fail compilation)
- **test_errors.nlang** - Various syntax/semantic errors
- **test_strict_order.nlang** - Tests declaration order enforcement
- **test_wrong_order.nlang** - Invalid dependency order (should fail)

---

## Creating Your Own Networks

### Template
```netlang
network YourNetworkName {
    // 1. Specify input dimensions
    input(shape: [Height, Width, Channels])
    
    // 2. Link weight file (.nwf format)
    weights("models/your_weights.nwf")
    
    // 3. Define layer-by-layer dataflow
    x = Conv2D(filters: F, kernel: [K, K], padding: P, activation: relu) from input
    x = MaxPool(pool: [2, 2], stride: 2) from x
    // ... more layers ...
    x = Flatten() from x
    x = Dense(units: N, activation: softmax) from x
    
    // 4. Done! Output is implicit (last layer)
}
```

### Naming Convention
- **Network name:** CamelCase architecture name (e.g., `LeNet5`, `ResNet50`)
- **Weight file:** `architecture_dataset.nwf` (e.g., `lenet5_mnist.nwf`, `vgg16_imagenet.nwf`)
- **Example file:** Lowercase with underscores (e.g., `lenet5.nlang`, `simple_classifier.nlang`)

### Best Practices
1. **Add comments:** Explain dimensions at each layer
2. **Cite papers:** Include reference to original architecture paper
3. **Document dataset:** Specify expected input size and normalization
4. **Test shapes:** Run compiler to validate shape inference before training weights

---

## Troubleshooting

### "Shape mismatch" errors
- Check input dimensions match your data
- Verify kernel/pool sizes don't violate spatial constraints
- Use `padding: 1` for 3×3 convs to preserve dimensions

### "Weight file not found"
- Ensure `.nwf` file exists at specified path (relative to project root)
- Convert your trained model using `tools/convert_*.py` scripts
- Check file permissions

### Slow inference
- Ensure `-O3 -mavx2 -mfma` flags are used during compilation
- Check CPU actually has AVX2 support: `grep avx2 /proc/cpuinfo` (Linux)
- Profile hot loops with `perf` or `gprof`

---

## Contributing Examples

Want to add your network? Create a pull request with:
1. `.nlang` file with comprehensive comments
2. Paper citation (if applicable)
3. Expected accuracy on test dataset
4. `.nwf` weight file (or instructions to generate)
5. Update this README with details

**Wishlist:**
- ResNet-50 (residual connections)
- MobileNet (depthwise separable convolutions)
- EfficientNet (compound scaling)
- YOLO (object detection)
- U-Net (segmentation)

---

## License

All example networks follow their original papers' conventions. Weight files may have separate licenses depending on source.

---

**See Also:**
- [Main README](../README.md) - Project overview
- [QUICKSTART.md](../QUICKSTART.md) - Detailed getting started guide
- [docs/WEIGHT_FORMAT.md](../docs/WEIGHT_FORMAT.md) - Weight file specification
