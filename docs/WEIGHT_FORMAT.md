# NetLang Weight Format (.nwf) Specification

**Version:** 1.0  
**Purpose:** High-performance weight storage for CNN inference with mmap and AVX2 optimization  
**Target:** Intel Core i5-5200U (AVX2), single-image inference

---

## Design Goals

1. **Zero-copy loading**: Direct mmap access without parsing
2. **AVX2-optimized**: 64-byte alignment for vectorized operations
3. **Cache-friendly**: Sequential memory layout, minimal indirection
4. **Portable**: Little-endian, IEEE-754 float32
5. **Extensible**: Reserved fields for future features

---

## File Structure

```
┌────────────────────────────────────────────────────────┐
│ FILE HEADER (256 bytes)                                │
├────────────────────────────────────────────────────────┤
│ LAYER METADATA TABLE (N × 64 bytes, 64-byte aligned)  │
├────────────────────────────────────────────────────────┤
│ WEIGHT DATA BLOB (64-byte aligned)                     │
└────────────────────────────────────────────────────────┘
```

---

## 1. File Header (256 bytes)

| Offset | Size | Type    | Field          | Description                           |
|--------|------|---------|----------------|---------------------------------------|
| 0      | 4    | char[4] | magic          | Magic number: "NWGT" (0x4E574754)    |
| 4      | 4    | uint32  | version        | Format version: 1                     |
| 8      | 4    | uint32  | layer_count    | Number of layers with weights         |
| 12     | 8    | uint64  | total_size     | Total file size in bytes              |
| 20     | 4    | uint32  | alignment      | Data alignment (64 for AVX2)          |
| 24     | 4    | uint32  | dtype          | Data type: 0=float32, 1=float16       |
| 28     | 8    | uint64  | metadata_offset| Byte offset to metadata table (256)   |
| 36     | 8    | uint64  | data_offset    | Byte offset to weight data blob       |
| 44     | 212  | byte[]  | reserved       | Reserved for future use (must be 0)   |

**Total: 256 bytes**

---

## 2. Layer Metadata Entry (64 bytes per layer)

| Offset | Size | Type    | Field          | Description                           |
|--------|------|---------|----------------|---------------------------------------|
| 0      | 4    | uint32  | layer_type     | 0=Conv2D, 1=Dense, 2=BatchNorm        |
| 4      | 4    | uint32  | layer_id       | Layer index in network (0-based)      |
| 8      | 8    | uint64  | weight_offset  | Byte offset to weight data            |
| 16     | 8    | uint64  | weight_size    | Size of weight data in bytes          |
| 24     | 16   | uint32[4] | weight_shape | [dim0, dim1, dim2, dim3]              |
| 40     | 8    | uint64  | bias_offset    | Byte offset to bias data (0=none)     |
| 48     | 8    | uint64  | bias_size      | Size of bias data in bytes            |
| 56     | 4    | uint32  | bias_shape     | [dim0] (1D vector)                    |
| 60     | 4    | byte[]  | reserved       | Reserved (must be 0)                  |

**Total: 64 bytes (cache-line aligned)**

### Layer Type Codes
- `0` = Conv2D: weight_shape = [kernel_h, kernel_w, in_channels, out_channels]
- `1` = Dense: weight_shape = [in_features, out_features, 0, 0]
- `2` = BatchNorm: weight_shape = [channels, 0, 0, 0] (gamma), bias = beta
- `3` = LayerNorm: Similar to BatchNorm

---

## 3. Weight Data Blob

- **Alignment**: Every weight/bias array starts at 64-byte boundary
- **Layout**: Row-major (C-contiguous)
- **Data type**: IEEE-754 float32 (4 bytes per element)
- **Order**: Weights first, then biases (if present)

### Conv2D Weight Layout
```
Shape: [Kh, Kw, Cin, Cout]
Memory: weights[Kh][Kw][Cin][Cout] (row-major)
Access: weight[kh * Kw*Cin*Cout + kw * Cin*Cout + cin * Cout + cout]
```

### Dense Weight Layout
```
Shape: [In, Out]
Memory: weights[In][Out] (row-major)
Access: weight[in * Out + out]
```

### Padding
Between each weight/bias array, insert zero-padding to reach next 64-byte boundary:
```c
padded_size = (size + 63) & ~63;  // Round up to 64-byte multiple
```

---

## 4. Memory Mapping (Runtime)

### Loading Example (C)
```c
#include <sys/mman.h>
#include <fcntl.h>

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t layer_count;
    // ... rest of header
} NWFHeader;

void* load_weights(const char* path) {
    int fd = open(path, O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);
    
    // mmap entire file
    void* addr = mmap(NULL, sb.st_size, PROT_READ,
                      MAP_PRIVATE | MAP_POPULATE, fd, 0);
    
    // Sequential access hint
    madvise(addr, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED);
    
    close(fd);  // File descriptor no longer needed
    return addr;
}

// Access weights directly
NWFHeader* header = (NWFHeader*)addr;
LayerMetadata* metadata = (LayerMetadata*)(addr + header->metadata_offset);
float* conv1_weights = (float*)(addr + metadata[0].weight_offset);
float* conv1_biases = (float*)(addr + metadata[0].bias_offset);
```

---

## 5. Conversion from Existing Formats

### From PyTorch (.pth, .pt)
```python
import torch
import struct

model = torch.load('vgg16.pth')
weights = []

for name, param in model.state_dict().items():
    if 'weight' in name or 'bias' in name:
        weights.append({
            'data': param.cpu().numpy().astype('float32'),
            'shape': param.shape,
            'type': 'weight' if 'weight' in name else 'bias'
        })

# Write to .nwf format (see conversion scripts)
```

### From TensorFlow/Keras (.h5)
```python
import tensorflow as tf

model = tf.keras.models.load_model('resnet50.h5')
for layer in model.layers:
    weights = layer.get_weights()  # [W, b]
    # Convert to .nwf
```

### From ONNX (.onnx)
```python
import onnx

model = onnx.load('model.onnx')
for initializer in model.graph.initializer:
    # Extract weight tensors
    # Convert to .nwf
```

---

## 6. Example File Layout

**MNIST CNN (2 Conv layers, 2 Dense layers):**

```
Offset    | Content                         | Size
----------|---------------------------------|--------
0x0000    | Header                          | 256 B
0x0100    | Layer 0 metadata (Conv2D)       | 64 B
0x0140    | Layer 1 metadata (Conv2D)       | 64 B
0x0180    | Layer 2 metadata (Dense)        | 64 B
0x01C0    | Layer 3 metadata (Dense)        | 64 B
0x0200    | Conv1 weights [3,3,1,32]        | 1,152 B
0x0680    | Conv1 biases [32]               | 128 B (padded)
0x0700    | Conv2 weights [3,3,32,64]       | 73,728 B
0x12740   | Conv2 biases [64]               | 256 B (padded)
0x12840   | Dense1 weights [6272,128]       | 3,211,264 B
0x322840  | Dense1 biases [128]             | 512 B (padded)
0x322A40  | Dense2 weights [128,10]         | 5,120 B
0x323E40  | Dense2 biases [10]              | 64 B (padded)
```

**Total size: ~3.2 MB**

---

## 7. Advantages Over Alternatives

| Format      | Load Time | mmap | AVX2 Ready | Parsing |
|-------------|-----------|------|------------|---------|
| **NWF**     | **<1ms**  | ✅   | ✅         | None    |
| ONNX        | ~100ms    | ❌   | ❌         | Complex |
| PyTorch PT  | ~50ms     | ❌   | ❌         | pickle  |
| HDF5        | ~80ms     | Partial | ❌      | Heavy   |
| NumPy NPZ   | ~30ms     | ❌   | ❌         | zip     |

---

## 8. Validation

### Checksum (Future Enhancement)
Reserved bytes in header can store SHA-256 hash for integrity verification.

### Endianness
Current version assumes little-endian (x86-64). Future versions may include endianness flag.

---

## 9. Tools

- **Converters**: `tools/convert_pytorch.py`, `tools/convert_keras.py`, `tools/convert_onnx.py`
- **Inspector**: `tools/inspect_nwf.py` - Display file structure
- **Validator**: `tools/validate_nwf.py` - Check format compliance

---

## References

- AVX2 Programming: Intel Intrinsics Guide
- mmap optimization: Linux `mmap(2)` man page
- IEEE-754: Floating-point standard
