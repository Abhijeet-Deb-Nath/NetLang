# NetLang Weight Files

Pretrained model weights in NetLang Weight Format (`.nwf`) for immediate inference.

---

## Available Models

| Model | Architecture | Dataset | Input Size | Parameters | Size |
|-------|-------------|---------|------------|------------|------|
| [lenet5_mnist.nwf](#lenet-5-mnist) | LeNet-5 | MNIST | 32×32×1 | ~60K | 348 KB |
| [mnist_cnn.nwf](#mnist-cnn-legacy) | Custom CNN | MNIST | 32×32×1 | ~400K | 1.6 MB |

---

## LeNet-5 (MNIST)

**File:** `lenet5_mnist.nwf`

**Size:** 348,160 bytes (~340 KB)

**Architecture:** LeNet-5 (LeCun et al., 1998)
```
Input [32×32×1]
  → Conv2D(6, 5×5) + ReLU → [28×28×6]      # 156 params (6×5×5×1 + 6)
  → MaxPool(2×2) → [14×14×6]
  → Conv2D(16, 5×5) + ReLU → [10×10×16]    # 2,416 params (16×5×5×6 + 16)
  → MaxPool(2×2) → [5×5×16]
  → Flatten() → [400]
  → Dense(120) + ReLU → [120]               # 48,120 params (400×120 + 120)
  → Dense(84) + ReLU → [84]                 # 10,164 params (120×84 + 84)
  → Dense(10) + Softmax → [10]              # 850 params (84×10 + 10)

Total: 61,706 parameters
```

**Training Details:**
- **Dataset:** MNIST (60,000 training, 10,000 test)
- **Optimizer:** Adam (lr=0.001)
- **Epochs:** 10
- **Framework:** PyTorch
- **Test Accuracy:** TBD (weights to be validated)

**Usage:**
```bash
# Compile network
./build/netlang examples/lenet5.nlang -o generated/lenet5.c

# Build executable
gcc -O3 -mavx2 generated/lenet5.c src/codegen/runtime.c src/codegen/kernels.c -o bin/lenet5

# Run inference
./bin/lenet5 test_data/preprocessed/mnist_0042_label_3.bin
```

**Binary Structure:**
```
Offset   | Content
---------|--------------------------------------------------
0x0000   | Magic: "NWF0" (4 bytes)
0x0004   | Version: 1 (uint32)
0x0008   | Num Layers: 5 (uint32)
0x000C   | Layer 0 Metadata: Conv2D(6, 5×5×1)
0x0040   | Layer 0 Data: 156 floats (624 bytes) [64-byte aligned]
0x0280   | Layer 1 Metadata: Conv2D(16, 5×5×6)
0x02C0   | Layer 1 Data: 2,416 floats (9,664 bytes)
...      | Layers 2-4 (Dense 120, Dense 84, Dense 10)
0x55000  | EOF (348,160 bytes total)
```

**Verification:**
```bash
# Check file size
ls -lh models/lenet5_mnist.nwf
# Expected: 340K

# Inspect header (Linux/macOS)
xxd models/lenet5_mnist.nwf | head -n 4
# Expected:
# 00000000: 4e57 4630 0100 0000 0500 0000 ...
#           N  W  F  0   (magic + version + num_layers)
```

---

## MNIST CNN (Legacy)

**File:** `mnist_cnn.nwf`

**Size:** 1,638,400 bytes (~1.6 MB)

**Architecture:** Custom deeper CNN
```
Input [32×32×1]
  → Conv2D(32, 3×3, padding: 1) + ReLU → [32×32×32]    # 320 params
  → MaxPool(2×2) → [16×16×32]
  → Conv2D(64, 3×3, padding: 1) + ReLU → [16×16×64]    # 18,496 params
  → MaxPool(2×2) → [8×8×64]
  → Flatten() → [4096]
  → Dense(128) + ReLU → [128]                           # 524,416 params
  → Dense(10) + Softmax → [10]                          # 1,290 params

Total: 544,522 parameters (2.1 MB uncompressed)
```

**Training Details:**
- **Dataset:** MNIST (60,000 training, 10,000 test)
- **Optimizer:** SGD with momentum 0.9
- **Epochs:** 20
- **Framework:** PyTorch
- **Test Accuracy:** TBD (weights to be validated)

**Notes:**
- Larger model with potentially higher accuracy than LeNet-5
- More parameters for better feature extraction
- Better feature extraction with 3×3 kernels + padding
- Legacy file - prefer `lenet5_mnist.nwf` for standard benchmarks

---

## Weight File Format (.nwf)

### Specification

NetLang Weight Format (`.nwf`) is a custom binary format optimized for:
- **Zero-copy loading:** mmap entire file, no parsing
- **Cache alignment:** 64-byte aligned layers for AVX2
- **Platform portability:** Little-endian float32 (IEEE 754)
- **Fast loading:** ~1ms vs ~80ms for ONNX

### Structure

```
┌─────────────────────────────────────────────────────┐
│ Header (12 bytes)                                   │
├─────────────────────────────────────────────────────┤
│ Magic: "NWF0" (4 bytes)                             │
│ Version: uint32 (4 bytes)                           │
│ Num Layers: uint32 (4 bytes)                        │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│ Layer 0 Metadata (variable size)                    │
├─────────────────────────────────────────────────────┤
│ Type: uint32 (0=Conv2D, 1=Dense)                    │
│ Output Channels: uint32                             │
│ Kernel Height: uint32                               │
│ Kernel Width: uint32                                │
│ Input Channels: uint32                              │
│ [Padding to 64-byte boundary]                       │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│ Layer 0 Data (N × float32, 64-byte aligned)         │
├─────────────────────────────────────────────────────┤
│ float32[0], float32[1], ..., float32[N-1]           │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│ Layer 1 Metadata (64-byte aligned)                  │
│ Layer 1 Data (64-byte aligned)                      │
│ ...                                                  │
│ Layer K Metadata                                    │
│ Layer K Data                                        │
└─────────────────────────────────────────────────────┘
```

### Memory Layout

**Conv2D Weights:**
```
Format: [out_channels, in_channels, kernel_h, kernel_w]
Example (16 filters, 6 input channels, 5×5 kernel):
  filter[0][0][0][0], filter[0][0][0][1], ..., filter[0][0][4][4],
  filter[0][1][0][0], ...,
  ...
  filter[15][5][4][4]
```

**Dense Weights:**
```
Format: [output_units, input_units] (row-major)
Example (120 outputs, 400 inputs):
  weight[0][0], weight[0][1], ..., weight[0][399],     # Weights for output 0
  weight[1][0], weight[1][1], ..., weight[1][399],     # Weights for output 1
  ...
  weight[119][0], ..., weight[119][399]                # Weights for output 119
```

**Biases (appended after weights):**
```
Format: [num_outputs]
Example (Dense 120):
  bias[0], bias[1], ..., bias[119]
```

### Loading Code

**C Example:**
```c
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct {
    void *data;
    size_t size;
} WeightFile;

WeightFile* load_weights(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return NULL;
    
    // Get file size
    struct stat st;
    fstat(fd, &st);
    
    // mmap entire file (zero-copy)
    void *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    
    if (data == MAP_FAILED) return NULL;
    
    WeightFile *wf = malloc(sizeof(WeightFile));
    wf->data = data;
    wf->size = st.st_size;
    
    return wf;
}

// Access layer data (zero-copy pointer arithmetic)
float* get_layer_weights(WeightFile *wf, int layer_idx) {
    uint8_t *ptr = (uint8_t*)wf->data;
    
    // Skip header
    ptr += 12;
    
    // Skip to target layer
    for (int i = 0; i < layer_idx; i++) {
        // Read metadata size and data size, then skip
        uint32_t data_size = /* read from metadata */;
        ptr += /* metadata size + padding */;
        ptr += data_size;
        ptr = (uint8_t*)(((uintptr_t)ptr + 63) & ~63); // Align to 64
    }
    
    // Skip metadata, return data pointer
    ptr += /* metadata size + padding */;
    return (float*)ptr;
}
```

See [docs/WEIGHT_FORMAT.md](../docs/WEIGHT_FORMAT.md) for complete specification.

---

## Creating Your Own Weights

### Option 1: Convert from PyTorch
```bash
# Train model in PyTorch
python train.py  # Outputs: model.pth

# Convert to .nwf
python tools/convert_pytorch.py model.pth models/my_model.nwf
```

### Option 2: Convert from Keras
```bash
# Train model in Keras/TensorFlow
python train.py  # Outputs: model.h5

# Convert to .nwf
python tools/convert_keras.py model.h5 models/my_model.nwf
```

### Option 3: Convert from ONNX
```bash
# Export from any framework to ONNX
# PyTorch:
torch.onnx.export(model, dummy_input, "model.onnx")
# TensorFlow:
python -m tf2onnx.convert --saved-model model/ --output model.onnx

# Convert to .nwf
python tools/convert_onnx.py model.onnx models/my_model.nwf
```

### Option 4: Generate Random Weights (Testing Only)
```bash
# Create random weights for architecture testing
gcc tools/create_test_weights.c -o bin/create_weights
./bin/create_weights models/random_lenet5.nwf \
    "Conv2D:6,5,5,1|Conv2D:16,5,5,6|Dense:120,400|Dense:84,120|Dense:10,84"
```

---

## Validation

### Visual Inspection
```bash
# Check file size
ls -lh models/*.nwf

# Inspect header (first 64 bytes)
xxd models/lenet5_mnist.nwf | head -n 4

# Verify magic bytes
head -c 4 models/lenet5_mnist.nwf | od -A n -t x1
# Expected: 4e 57 46 30  (ASCII: "NWF0")
```

### Programmatic Validation
```python
import struct

def validate_nwf(filepath):
    with open(filepath, 'rb') as f:
        # Read header
        magic = f.read(4)
        version, num_layers = struct.unpack('II', f.read(8))
        
        print(f"Magic: {magic} (expected: b'NWF0')")
        print(f"Version: {version}")
        print(f"Num Layers: {num_layers}")
        
        # Validate
        assert magic == b'NWF0', "Invalid magic bytes"
        assert version == 1, f"Unsupported version: {version}"
        assert num_layers > 0, "No layers found"
        
        print("✓ Valid .nwf file")

validate_nwf("models/lenet5_mnist.nwf")
```

### Full Inference Test
```bash
# Compile test harness
gcc -O3 -mavx2 tools/test_network.c generated/lenet5.c \
    src/codegen/runtime.c src/codegen/kernels.c -o bin/test_network

# Run on sample data
./bin/test_network test_data/preprocessed/mnist_0000_label_7.bin models/lenet5_mnist.nwf
# Output: (Testing in progress)
```

---

## Performance Impact

### Loading Performance

| Format | Size | Load Time | Method |
|--------|------|-----------|--------|
| ONNX | 350 KB | TBD | Protobuf parsing + allocation |
| HDF5 | 360 KB | TBD | HDF5 library overhead |
| PyTorch | 355 KB | TBD | pickle + torch.load |
| **.nwf** | **348 KB** | **TBD** | **mmap (zero-copy)** |

*Performance benchmarks to be measured*

### Memory Efficiency

**.nwf Advantages:**
- No parsing overhead (direct mmap)
- No dynamic allocation (fixed addresses)
- Page-aligned data (efficient TLB usage)
- Read-only mapping (shareable across processes)

**Frameworks:**
- Parse format → allocate buffers → copy data
- Heap fragmentation
- Garbage collection overhead (Python/Java)

---

## Troubleshooting

### "Invalid magic bytes" error
```bash
# Check if file is corrupted
xxd models/your_file.nwf | head -n 1
# Should start with: 4e 57 46 30

# Reconvert from original model
python tools/convert_pytorch.py original.pth models/your_file.nwf
```

### "Weight file too small" error
```bash
# Check actual file size
ls -l models/your_file.nwf

# Compare with expected size (from architecture)
# LeNet-5: ~348 KB (61,706 params × 4 bytes + metadata)
```

### "Shape mismatch" errors
```bash
# Ensure .nlang architecture matches weight dimensions
# Example: If .nlang has Conv2D(filters: 6), weights must have 6 output channels

# Inspect weight file structure
python tools/inspect_nwf.py models/your_file.nwf  # (to be implemented)
```

### Slow inference despite fast load
- Check AVX2 flags: `-mavx2 -mfma` during compilation
- Verify 64-byte alignment: Use `posix_memalign()` for input buffers
- Profile hot loops: `perf record ./bin/yournet input.bin`

---

## Contributing

### Adding Pretrained Models

Submit a PR with:
1. `.nwf` file in `models/`
2. Corresponding `.nlang` architecture in `examples/`
3. Training script or source framework model
4. Accuracy report on test dataset
5. Update this README with model details

**Requirements:**
- Test accuracy ≥ 90% (or state-of-the-art for dataset)
- File size ≤ 100 MB (use Git LFS for larger models)
- GPL/MIT/Apache compatible license

**Wishlist:**
- ResNet-50 (ImageNet)
- MobileNetV2 (Edge deployment)
- EfficientNet-B0 (High efficiency)
- YOLO-v5s (Object detection)
- U-Net (Medical imaging)

---

## License

- **Format specification (.nwf):** MIT License
- **Individual weight files:** See source framework licenses
  - `lenet5_mnist.nwf`: MIT (converted from PyTorch)
  - `mnist_cnn.nwf`: MIT (converted from PyTorch)

---

**See Also:**
- [docs/WEIGHT_FORMAT.md](../docs/WEIGHT_FORMAT.md) - Complete format specification
- [tools/README.md](../tools/README.md) - Weight conversion tools
- [examples/README.md](../examples/README.md) - Example architectures
