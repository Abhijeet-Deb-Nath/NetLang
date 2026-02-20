# NetLang User Guide

Practical guide to using NetLang for neural network compilation and inference.

> **Note:** For a 5-minute quick start, see [QUICKSTART.md](../QUICKSTART.md)

---

## Overview

NetLang compiles CNN architectures into optimized C code. The typical workflow:

1. Write `.nlang` architecture definition
2. Convert trained weights to `.nwf` format  
3. Compile architecture → C code
4. Compile C code → executable
5. Run inference

---

## Basic Workflow

### 1. Define Architecture (.nlang file)

```netlang
network LeNet5 {
    input(shape: [28, 28, 1])
    weights("models/lenet5_mnist.nwf")
    
    x = Conv2D(filters: 6, kernel: [5, 5], activation: relu) from input
    x = MaxPool(pool: [2, 2], stride: 2) from x
    x = Flatten() from x
    output = Dense(units: 10, activation: softmax) from x
}
```

### 2. Convert Weights (.pth/.onnx → .nwf)

```bash
# From PyTorch
python tools/convert_pytorch.py model.pth models/model.nwf

# From ONNX
python tools/convert_onnx.py model.onnx models/model.nwf
```

### 3. Compile Architecture (.nlang → .c)

```bash
bin/netlang_compiler architecture/lenet5.nlang -o generated/lenet5.c
```

### 4. Build Executable (.c → binary)

```bash
gcc -O3 -mavx2 generated/lenet5.c -o lenet5_infer -lm
```

### 5. Run Inference

```bash
./lenet5_infer input.bin
```

---

## Network Definition Language

### Basic Syntax

```netlang
network NetworkName {
    // Configuration
    input(shape: [height, width, channels])
    weights("path/to/weights.nwf")
    
    // Layer definitions
    variable = LayerType(params...) from source
    
    // Multiple statements
    x = Conv2D(...) from input
    y = MaxPool(...) from x
    output = Dense(...) from y
}
```

### Configuration Directives

**`input(shape: [H, W, C])`** - Define input tensor dimensions

```netlang
input(shape: [224, 224, 3])  // ImageNet size
input(shape: [28, 28, 1])    // MNIST size
input(shape: [32, 32, 3])    // CIFAR-10 size
```

**`weights("path.nwf")`** - Specify weight file path

```netlang
weights("models/vgg16.nwf")
weights("../pretrained/resnet50.nwf")
```

### Layer Types

#### Conv2D - 2D Convolution

```netlang
x = Conv2D(
    filters: 64,              // Number of output channels
    kernel: [3, 3],           // Kernel size [height, width]
    stride: 1,                // Stride (default: 1)
    padding: 1,               // Padding (default: 0)
    activation: relu          // relu, sigmoid, tanh, none
) from input
```

**Output shape:** `[(H+2P-K)/S + 1, (W+2P-K)/S + 1, F]`

#### Dense - Fully Connected Layer

```netlang
x = Dense(
    units: 1000,              // Number of output neurons
    activation: softmax       // Activation function
) from input
```

**Input:** 1D tensor `[N]`  
**Output:** 1D tensor `[units]`

#### MaxPool - Max Pooling

```netlang
x = MaxPool(
    pool: [2, 2],             // Pool size [height, width]
    stride: 2                 // Stride (default: same as pool size)
) from input
`` Supported Layers

| Layer | Syntax | Description |
|-------|--------|-------------|
| **Conv2D** | `Conv2D(filters: 64, kernel: [3,3], stride: 1, padding: 1, activation: relu)` | 2D convolution with AVX2 optimization |
| **Dense** | `Dense(units: 128, activation: relu)` | Fully connected layer |
| **MaxPool** | `MaxPool(pool: [2,2], stride: 2)` | Max pooling |
| **AvgPool** | `AvgPool(pool: [2,2], stride: 2)` | Average pooling |
| **GlobalAvgPool** | `GlobalAvgPool()` | Global average (spatial dims → scalar) |
| **Flatten** | `Flatten()` | Reshape [H,W,C] → [H×W×C] |
| **BatchNorm** | `BatchNorm()` | Batch normalization |
| **Dropout** | `Dropout(rate: 0.5)` | Dropout (no-op in inference) |

### Converting PyTorch Models

```python
# tools/convert_pytorch.py
import torch
import struct

def convert_pytorch_to_nwf(model_path, output_path):
    """
    Convert PyTorch model to NetLang weight format.
    
    Args:
        model_path: Path to .pth file
        output_path: Path to output .nwf file
    """
    # Load model
    state_dict = torch.load(model_path, map_location='cpu')
    
    with open(output_path, 'wb') as f:
        # Write header
        f.write(b'NWF')           # Magic number
        f.write(struct.pack('I', 1))  # Version
        
        # Write each layer's weights
        for name, tensor in state_dict.items():
            weights = tensor.numpy().flatten()
            f.write(struct.pack(f'{len(weights)}f', *weights))

# Usage
convert_pytorch_to_nwf('lenet5.pth', 'lenet5.nwf')
```

**Full script:** [tools/convert_pytorch.py](../tools/convert_pytorch.py)

### Converting ONNX Models

```bash
python tools/convert_onnx.py \
    --input model.onnx \
    --output model.nwf
```

### Converting Keras Models

```python
# tools/convert_keras.py
from tensorflow import keras
import numpy as np

model = keras.models.load_model('model.h5')

# Extract weights
weights = []
for layer in model.layers:
    w = layer.get_weights()
    weights.extend(w)

# Save to .nwf format
save_nwf(weights, 'model.nwf')
```

---

## Input Data Preparation

### Image Preprocessing

NetLang expects raw binary input (float32, row-major).

**Python preprocessing:**

Use the provided tools to convert trained models:

```bash
# PyTorch → .nwf
python tools/convert_pytorch.py model.pth models/output.nwf

# ONNX → .nwf
python tools/convert_onnx.py model.onnx models/output.nwf

# Keras → .nwf
python tools/convert_keras.py model.h5 models/output.nwf
```

See [tools/README.md](../tools/README.md) for format details.f report
```

---

## Example: Complete MNIST Pipeline

### 1. Download MNIST Data

```python
# tools/download_mnist_for_netlang.py
from torchvision import datasets

# Download MNIST
datasets.MNIST('./data', train=False, download=True)
```

### 2. Preprocess Test Images

```python
# tools/preprocess.py
import numpy as np
from PIL import Image

# Load MNIST test image
from torchvision import datasets
mnist = datasets.MNIST('./data', train=False)
img, label = mnist[0]

# Convert to float32, normalize
arr = np.array(img, dtype=np.float32) / 255.0
arr.tofile('test_input.bin')
## Performance Tips

**Compiler flags for maximum speed:**
```bash
gcc -O3 -march=native -mavx2 -ffast-math -funroll-loops generated/network.c -o infer -lm
```

**Benchmarking:**
```bash
# Measure inference time
time ./infer input.bin

# Test accuracy on dataset
python test_accuracy.py --network arch.nlang --num-samples 500
# Run inference
./lenet5_infer test_input.bin
```

**Expected output:**

```
Class probabilities:
0: 0.0001
1: 0.0002
2: 0.0010
3: 0.9850  ← Predicted class: 3
4: 0.0005
5: 0.0120
6: 0.0001
7: 0.0008
8: 0.0002
9: 0.0001

Predicted: 3 (confidence: 98.50%)
```

---

## Testing Accuracy

Use the provided test script to validate accuracy on multiple images:
## Complete Example: MNIST

```bash
# 1. Get MNIST data
python tools/download_mnist_for_netlang.py

# 2. Convert trained PyTorch model
python tools/convert_pytorch.py lenet5.pth models/lenet5_mnist.nwf

# 3. Compile architecture
bin/netlang_compiler architecture/lenet5.nlang -o generated/lenet5.c

# 4. Build executable
gcc -O3 -mavx2 generated/lenet5.c -o lenet5_infer -lm

# 5. Run inference
./lenet5_infer test_data/mnist_0.bin

# 6. Test accuracy
python test_accuracy.py --network architecture/lenet5.nlang --num-samples 500
# Expected: ~97% accuracy, ~12ms per image## Troubleshooting

**"undefined reference to `__mm256_loadu_ps`"**  
→ Add `-mavx2` flag to gcc

**"Cannot open weight file"**  
→ Check `weights()` path is correct and relative to execution directory

**"shape mismatch at layer X"**  
→ Verify `.nlang` architecture matches trained model dimensions

**Segmentation fault**  
→ Check input file size, verify weight file integrity

**Slow inference**  
→ Use `-O3 -march=native -mavx2` flags, verify AVX2 support with `lscpu`

---

## Advanced Topics

**Custom layers:** Extend lexer, parser, semantic analyzer, and codegen  
**Multi-output networks:** Modify generated code to handle multiple outputs  
**Batch processing:** Loop over single-image inference in main()

See [ARCHITECTURE.md](ARCHITECTURE.md) for compiler internals and [IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md) for extension points.

---

## References

- [QUICKSTART.md](../QUICKSTART.md) - 5-minute quick start
- [ARCHITECTURE.md](ARCHITECTURE.md) - Compiler design
- [WEIGHT_FORMAT.md](WEIGHT_FORMAT.md) - .nwf specification
- [OPTIMIZATION_ROADMAP.md](OPTIMIZATION_ROADMAP.md) - Performance roadmap
- [tools/README.md](../tools/README.md) - Utility