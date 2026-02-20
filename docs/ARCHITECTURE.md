# NetLang Compiler Architecture

A comprehensive guide to the NetLang compiler design and implementation.

---

## Overview

NetLang is a domain-specific language (DSL) compiler that transforms high-level CNN architecture definitions into optimized x86-64 machine code. It provides ahead-of-time compilation with automatic shape inference, type checking, and SIMD vectorization.

### Design Philosophy

**Objectives:**
1. **Simplicity** - Single-file C output, no runtime dependencies
2. **Performance** - CPU-optimized inference with AVX2 vectorization
3. **Portability** - Standard C output compilable on any x86-64 platform
4. **Determinism** - All optimization at compile-time, no JIT overhead

### Compilation Pipeline

```
┌──────────────────────────────────────────────────────────────────┐
│                     INPUT: .nlang Architecture                    │
└───────────────────────────────┬──────────────────────────────────┘
                                │
                    ┌───────────▼──────────┐
                    │  PHASE 1: FRONTEND   │
                    │  Lexer → Parser      │
                    │  Output: AST         │
                    └───────────┬──────────┘
                                │
                    ┌───────────▼───────────┐
                    │ PHASE 2: SEMANTIC     │
                    │ Shape Inference       │
                    │ Type Checking         │
                    │ Output: Annotated AST │
                    └───────────┬───────────┘
                                │
                    ┌───────────▼──────────┐
                    │ PHASE 3: CODEGEN     │
                    │ C Code Generation    │
                    │ Output: .c file      │
                    └───────────┬──────────┘
                                │
                    ┌───────────▼──────────┐
                    │   C Compiler (GCC)   │
                    │   Output: Executable │
                    └──────────────────────┘
```

---

## Phase 1: Frontend

### Lexical Analysis (Flex)

**File:** [src/lexer/net_lang.l](../src/lexer/net_lang.l)

Tokenizes the input `.nlang` file into a stream of tokens.

**Token Categories:**
- **Keywords:** `network`, `module`, `input`, `weights`, `from`
- **Layer Types:** `Conv2D`, `Dense`, `MaxPool`, `Flatten`, `BatchNorm`, `Dropout`
- **Activations:** `relu`, `sigmoid`, `tanh`, `softmax`, `none`
- **Operators:** `=`, `,`, `:`, `[`, `]`, `{`, `}`, `(`, `)`
- **Literals:** integers, floats, strings, identifiers

**Example:**
```netlang
x = Conv2D(filters: 32, kernel: [3, 3]) from input
```

**Tokens:**
```
IDENTIFIER(x), EQUALS, CONV2D, LPAREN, FILTERS, COLON, 
INTEGER(32), COMMA, KERNEL, COLON, LBRACKET, INTEGER(3), 
COMMA, INTEGER(3), RBRACKET, RPAREN, FROM, IDENTIFIER(input)
```

### Syntax Analysis (Bison)

**File:** [src/parser/net_lang.y](../src/parser/net_lang.y)

Parses token stream into Abstract Syntax Tree (AST) using context-free grammar.

**Grammar Structure:**
```
program        → network_def | module_def
network_def    → NETWORK ID { config_list layer_list }
config_list    → config config_list | ε
config         → input(...) | weights(...)
layer_list     → layer layer_list | ε
layer          → ID = layer_op from ID
layer_op       → Conv2D(...) | Dense(...) | MaxPool(...) | ...
```

**AST Node Types:**
- `NetworkNode` - Root of network definition
- `LayerNode` - Individual layer operation
- `ConfigNode` - Input/weights configuration
- `ExprNode` - Parameter expressions

---

## Phase 2: Semantic Analysis

### Symbol Table Management

**File:** [src/semantic/symbol_table.c](../src/semantic/symbol_table.c)

Tracks all variables (tensors) in the network graph.

**Symbol Table Entry:**
```c
typedef struct SymbolEntry {
    char *name;              // Variable name
    TensorShape shape;       // [batch, height, width, channels]
    DataType dtype;          // FLOAT32, INT32, etc.
    bool is_input;           // Input tensor flag
    bool is_output;          // Output tensor flag
    LayerNode *definition;   // Defining layer
} SymbolEntry;
```

**Operations:**
- `symbol_table_insert(name, shape, dtype)` - Add new variable
- `symbol_table_lookup(name)` - Find variable by name
- `symbol_table_check_defined(name)` - Verify variable exists

### Shape Inference

**File:** [src/semantic/semantic.c](../src/semantic/semantic.c)

Automatically computes output shapes for all layers based on:
1. Input tensor shape
2. Layer parameters (kernel size, stride, padding)
3. Layer type

**Shape Rules:**

**Conv2D:**
```
output_h = (input_h - kernel_h + 2*padding) / stride + 1
output_w = (input_w - kernel_w + 2*padding) / stride + 1
output_c = filters
```

**MaxPool:**
```
output_h = (input_h - pool_h) / stride + 1
output_w = (input_w - pool_w) / stride + 1
output_c = input_c  (unchanged)
```

**Dense:**
```
output_shape = [batch, units]
```

**Flatten:**
```
output_shape = [batch, input_h * input_w * input_c]
```

### Type Checking

**File:** [src/semantic/type_checker.c](../src/semantic/type_checker.c)

Validates network correctness before code generation.

**Checks Performed:**
1. **Use-before-definition** - All variables defined before use
2. **Shape compatibility** - Layer inputs match expected dimensions
3. **Parameter validity** - Kernel sizes, strides are positive
4. **Network completeness** - Input and output layers defined
5. **No dangling layers** - All layers connected to graph

**Error Examples:**
```netlang
x = Conv2D(...) from undefined_var   // Error: undefined_var not found
x = Dense(units: 10) from conv_layer // Error: Dense expects 2D, got 4D
```

---

## Phase 3: Code Generation

### Code Generator Core

**File:** [src/codegen/codegen.c](../src/codegen/codegen.c)

Generates C code with embedded neural network inference.

**Generated Code Structure:**
```c
// 1. Headers and includes
#include <stdio.h>
#include <immintrin.h>  // AVX2 intrinsics

// 2. Weight declarations (external linkage)
extern float conv1_weights[...];
extern float conv1_bias[...];

// 3. Activation functions
static inline float relu(float x) { return x > 0 ? x : 0; }

// 4. Layer implementations (specialized per network)
void conv2d_layer1(float *input, float *output) {
    // Specialized Conv2D for layer 1 dimensions
    // Uses AVX2 for 8-wide SIMD
}

// 5. Network inference function
void infer(float *input_data, float *output_data) {
    // Allocate intermediate buffers
    float layer1_out[28*28*6];
    float layer2_out[14*14*6];
    
    // Execute layers in sequence
    conv2d_layer1(input_data, layer1_out);
    maxpool_layer2(layer1_out, layer2_out);
    // ...
}

// 6. Main function (load weights, run inference)
int main() {
    // mmap weight file
    // Load input image
    // Call infer()
    // Print results
}
```

### Kernel Implementations

**File:** [src/codegen/kernels.c](../src/codegen/kernels.c)

Optimized computation kernels for each layer type.

**Conv2D Kernel (Current Implementation):**
```c
void emit_conv2d_kernel(FILE *fp, LayerNode *layer) {
    int ih = layer->input_shape.h;
    int iw = layer->input_shape.w;
    int ic = layer->input_shape.c;
    int filters = layer->filters;
    int kh = layer->kernel_h;
    int kw = layer->kernel_w;
    
    fprintf(fp, "void %s(float *input, float *output) {\n", layer->name);
    fprintf(fp, "    for (int f = 0; f < %d; f++) {\n", filters);
    fprintf(fp, "        for (int oh = 0; oh < %d; oh++) {\n", oh);
    fprintf(fp, "            for (int ow = 0; ow < %d; ow++) {\n", ow);
    
    // AVX2 vectorized accumulation
    fprintf(fp, "                __m256 sum = _mm256_setzero_ps();\n");
    fprintf(fp, "                for (int kh = 0; kh < %d; kh++) {\n", kh);
    fprintf(fp, "                    for (int kw = 0; kw < %d; kw++) {\n", kw);
    fprintf(fp, "                        for (int c = 0; c < %d; c += 8) {\n", ic);
    
    // SIMD operations
    fprintf(fp, "                            __m256 in = _mm256_loadu_ps(&input[...]);\n");
    fprintf(fp, "                            __m256 w = _mm256_loadu_ps(&weights[...]);\n");
    fprintf(fp, "                            sum = _mm256_add_ps(sum, _mm256_mul_ps(in, w));\n");
    
    // Horizontal sum and store
    fprintf(fp, "                        }\n");
    fprintf(fp, "                    }\n");
    fprintf(fp, "                }\n");
    fprintf(fp, "                output[...] = horizontal_sum(sum) + bias[f];\n");
    fprintf(fp, "            }\n");
    fprintf(fp, "        }\n");
    fprintf(fp, "    }\n");
    fprintf(fp, "}\n");
}
```

**Current Optimizations:**
- ✅ AVX2 vectorization (8-wide float operations)
- ✅ Loop unrolling hints
- ✅ Inline activation functions

**Missing Optimizations (Future Work):**
- ❌ Operator fusion (Conv+ReLU combined)
- ❌ Cache blocking/tiling
- ❌ Multi-threading (OpenMP)
- ❌ FMA instructions
- ❌ Im2col transformation

### Runtime System

**File:** [src/codegen/runtime.c](../src/codegen/runtime.c)

Provides weight loading and memory management utilities.

**Weight Loading (mmap-based):**
```c
void load_weights(const char *filename) {
    int fd = open(filename, O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);
    
    // Memory-map weight file (zero-copy)
    void *mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    // Parse .nwf header
    NWFHeader *header = (NWFHeader*)mapped;
    
    // Set weight pointers (no copy needed!)
    conv1_weights = (float*)(mapped + header->conv1_offset);
    conv1_bias = (float*)(mapped + header->conv1_bias_offset);
    // ...
}
```

**Benefits:**
- ~1ms loading time (vs ~100ms for PyTorch)
- No memory copy overhead
- OS handles paging automatically

---

## Data Structures

### AST Node Hierarchy

```c
// Base node type
typedef enum {
    NODE_NETWORK,
    NODE_MODULE,
    NODE_LAYER,
    NODE_CONFIG
} NodeType;

// Layer node (main building block)
typedef struct LayerNode {
    NodeType type;
    char *name;              // Variable name
    char *op;                // Layer operation (Conv2D, Dense, etc.)
    
    // Input/output connections
    char *input_from;        // Source variable name
    TensorShape input_shape;
    TensorShape output_shape;
    
    // Layer-specific parameters
    int filters;             // Conv2D, DepthwiseConv
    int kernel_h, kernel_w;  // Conv2D, MaxPool
    int stride;              // Conv2D, MaxPool
    int units;               // Dense
    char *activation;        // relu, sigmoid, etc.
    float dropout_rate;      // Dropout
    
    // Semantic analysis results
    bool shape_inferred;
    bool type_checked;
    
    struct LayerNode *next;  // Linked list of layers
} LayerNode;

// Network node (root)
typedef struct {
    char *name;              // Network name
    TensorShape input_shape; // Input dimensions
    char *weights_file;      // Path to .nwf file
    LayerNode *layers;       // Layer list
    int num_layers;
} NetworkNode;
```

---

## Weight File Format (.nwf)

NetLang uses a custom binary format for efficient weight loading.

**File Structure:**
```
┌────────────────────────────┐
│ Header (256 bytes)         │
│  - Magic number (0x4E5746) │
│  - Version                 │
│  - Layer count             │
│  - Offsets array           │
├────────────────────────────┤
│ Layer 1 Weights (aligned)  │
├────────────────────────────┤
│ Layer 1 Bias (aligned)     │
├────────────────────────────┤
│ Layer 2 Weights            │
├────────────────────────────┤
│ ...                        │
└────────────────────────────┘
```

**Advantages:**
- Memory-mapped loading (zero-copy)
- 64-byte alignment for AVX2 (faster loads)
- Direct pointer access (no deserialization)

See [WEIGHT_FORMAT.md](WEIGHT_FORMAT.md) for full specification.

---

## Supported Layers

| Layer | Input Dims | Output Dims | Parameters | Status |
|-------|-----------|-------------|------------|--------|
| Conv2D | [H, W, C] | [H', W', F] | filters, kernel, stride, padding, activation | ✅ |
| DepthwiseConv2D | [H, W, C] | [H', W', C] | kernel, stride, multiplier | ✅ |
| Dense | [N] | [M] | units, activation | ✅ |
| MaxPool | [H, W, C] | [H', W', C] | pool_size, stride | ✅ |
| AvgPool | [H, W, C] | [H', W', C] | pool_size, stride | ✅ |
| GlobalAvgPool | [H, W, C] | [C] | - | ✅ |
| Flatten | [H, W, C] | [H×W×C] | - | ✅ |
| BatchNorm | [H, W, C] | [H, W, C] | gamma, beta, mean, var | ✅ |
| Dropout | [any] | [same] | rate | ✅ (inference mode) |
| Concatenate | [H, W, C1], [H, W, C2] | [H, W, C1+C2] | axis | ✅ |
| Add | [H, W, C], [H, W, C] | [H, W, C] | - | ✅ |

---

## Activation Functions

| Activation | Formula | Implementation |
|-----------|---------|----------------|
| ReLU | max(0, x) | `x > 0 ? x : 0` |
| Sigmoid | 1/(1+e^-x) | `1.0f / (1.0f + expf(-x))` |
| Tanh | tanh(x) | `tanhf(x)` |
| Softmax | e^xi / Σe^xj | Stable impl with max subtraction |
| None | x | Pass-through |

---

## Error Handling

### Compile-Time Errors

**Semantic Errors:**
- Undefined variable usage
- Shape mismatch (e.g., Dense after Conv without Flatten)
- Missing input/weights configuration
- Invalid parameters (negative kernel size, etc.)

**Example:**
```
Error: Variable 'x' used before definition at line 5
Error: Shape mismatch: Dense expects [N], got [28, 28, 6] at line 8
Error: Missing input configuration in network 'LeNet5'
```

### Runtime Errors

**Weight Loading:**
- File not found
- Corrupted .nwf file (invalid magic number)
- Size mismatch (weights don't match architecture)

**Inference:**
- Memory allocation failure
- Invalid input dimensions

---

## Performance Characteristics

### Current Performance (LeNet5 on MNIST)

- **Compilation time:** ~50ms (.nlang → .c)
- **C compilation:** ~500ms (gcc -O3)
- **Weight loading:** ~1ms (mmap)
- **Inference time:** ~10-13ms per image (28×28×1)
- **Accuracy:** 97% on MNIST test set

### Comparison with Production Frameworks

| Framework | Inference Time | Relative Speed |
|-----------|---------------|----------------|
| NetLang (current) | 10-13ms | 1x baseline |
| ONNX Runtime | 0.23ms | **43x faster** |
| PyTorch (eager) | ~5ms | 2x faster |
| TensorFlow | ~3ms | 3x faster |

**Performance Gap Analysis:**
- **Main bottleneck:** Kernel implementation quality
- **Missing optimizations:** Operator fusion, cache blocking, multi-threading
- **Not architectural:** ONNX also specializes at runtime, no compile-time advantage lost

See [OPTIMIZATION_ROADMAP.md](OPTIMIZATION_ROADMAP.md) for improvement plan.

---

## Limitations

### Current Limitations

1. **CPU-only** - No GPU support (by design)
2. **Single-threaded** - No OpenMP parallelization yet
3. **Limited operator fusion** - Each layer is separate function
4. **No training** - Inference only (weights from PyTorch/TensorFlow)
5. **x86-64 only** - AVX2 required (Intel Core 4th gen+, AMD Excavator+)

### Architectural Constraints

1. **Static shapes** - Input dimensions must be known at compile time
2. **No dynamic control flow** - No if/while in network graph
3. **No dynamic batching** - Batch size = 1 (single image inference)

---

## Design Decisions

### Why Ahead-of-Time Compilation?

**Advantages:**
- Predictable performance (no JIT overhead)
- Smaller binaries (no JIT compiler in executable)
- Fast startup (no graph optimization at runtime)
- Better for embedded/edge deployment

**Trade-offs:**
- Must recompile for different input sizes
- No runtime graph optimization
- Larger executable per network (vs shared runtime)

### Why Custom Weight Format?

**Alternatives considered:**
- ONNX - Too heavy, requires protobuf parser
- HDF5 - Large library dependency
- NumPy .npy - Python-centric, not aligned

**Benefits of .nwf:**
- Zero-copy mmap loading
- AVX2-aligned (64-byte boundaries)
- Tiny parser (~100 lines of C)
- Cross-platform (little-endian assumed)

### Why C Output (not LLVM IR)?

**Rationale:**
- Portable - Any C compiler works
- Debuggable - Can inspect generated code
- Simple - No LLVM dependency
- Flexible - Can hand-tune generated code

**Trade-offs:**
- Rely on C compiler optimizer (gcc -O3)
- Can't do advanced optimizations (polyhedral, etc.)

---

## Future Architecture Enhancements

See [OPTIMIZATION_ROADMAP.md](OPTIMIZATION_ROADMAP.md) for detailed plan.

**High-priority:**
1. Operator fusion (Conv+ReLU, Conv+BN+ReLU)
2. Cache blocking for convolutions
3. Multi-threading with OpenMP
4. FMA instruction usage

**Medium-priority:**
1. Im2col convolution algorithm
2. Winograd convolution (for 3×3 kernels)
3. In-place operations (reduce memory)

**Low-priority:**
1. Dynamic batching support
2. ARM NEON backend
3. INT8 quantization

---

## References

- **Flex/Bison:** [GNU Flex Manual](https://www.gnu.org/software/flex/manual/)
- **AVX2:** [Intel Intrinsics Guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide/)
- **Conv Optimization:** Lavin & Gray (2016), "Fast Algorithms for Convolutional Neural Networks"
- **Weight Format:** Custom design, inspired by GGML/GGUF formats
