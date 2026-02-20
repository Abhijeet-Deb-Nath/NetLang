# Contributing to NetLang

Thank you for your interest in contributing to NetLang! This guide will help you understand the project structure and development workflow.

---

## Table of Contents

1. [Project Philosophy](#project-philosophy)
2. [Naming Conventions](#naming-conventions)
3. [Directory Structure](#directory-structure)
4. [Development Setup](#development-setup)
5. [Code Style](#code-style)
6. [Adding Features](#adding-features)
7. [Testing](#testing)
8. [Pull Request Process](#pull-request-process)

---

## Project Philosophy

NetLang is designed around these core principles:

1. **Compile-Time Optimization** - Maximize static analysis, minimize runtime overhead
2. **Transparent Performance** - Users should understand exactly what code is generated
3. **Framework Agnostic** - Support PyTorch, TensorFlow, ONNX via standardized .nwf format
4. **Edge-First** - Target CPU inference for embedded/edge devices
5. **Zero Dependencies** - Generated code should have no external runtime dependencies

**Target Use Case:** Deploy trained CNNs to embedded systems, IoT devices, or servers where low latency and minimal memory footprint are critical.

---

## Naming Conventions

### Critical: Architecture vs Dataset

**Understand the distinction:**
- **Architecture** = Network structure (LeNet-5, VGG-16, ResNet-50)
- **Dataset** = Training data (MNIST, ImageNet, CIFAR-10)
- **Model** = Architecture + Trained Weights

### File Naming Rules

| Type | Convention | Example | ❌ Wrong |
|------|-----------|---------|----------|
| Architecture file | `architecture_name.nlang` | `lenet5.nlang` | `mnist.nlang` |
| Weight file | `architecture_dataset.nwf` | `lenet5_mnist.nwf` | `mnist_weights.nwf` |
| Network name (in .nlang) | `CamelCaseArchitecture` | `LeNet5` | `MNIST_CNN` |
| Generated code | `generated_architecture.c` | `generated_lenet5.c` | `generated_mnist.c` |

**Rationale:**
- MNIST is a **dataset**, not an **architecture**
- LeNet-5 is an **architecture** (can be trained on any 32×32 dataset)
- "MNIST_CNN" is ambiguous (could be any CNN trained on MNIST)

### Network Name Guidelines

```netlang
// ✅ GOOD - Architecture name
network LeNet5 {
    input(shape: [32, 32, 1])
    weights("models/lenet5_mnist.nwf")  // Dataset specified in filename
    ...
}

// ❌ BAD - Dataset name used as architecture
network MNIST_CNN {
    weights("models/mnist_weights.nwf")
    ...
}

// ✅ GOOD - Custom architecture
network MyCustomCNN {
    input(shape: [32, 32, 3])
    weights("models/mycustomcnn_cifar10.nwf")
    ...
}
```

---

## Directory Structure

```
netlang/
├── src/                    # Compiler source (C)
│   ├── lexer/             # Tokenization (Flex)
│   │   └── net_lang.l     # Lexer rules
│   ├── parser/            # Syntax analysis (Bison)
│   │   └── net_lang.y     # Parser grammar
│   ├── ast/               # Abstract Syntax Tree
│   │   ├── ast.h/.c       # AST node definitions
│   ├── semantic/          # Type checking & shape inference
│   │   ├── semantic.h/.c
│   │   ├── symbol_table.h/.c
│   │   └── type_checker.h/.c
│   └── codegen/           # Code generation & runtime
│       ├── codegen.h/.c   # C code generator
│       ├── runtime.h/.c   # Weight loading (mmap)
│       └── kernels.h/.c   # AVX2 inference kernels
│
├── examples/               # 🎯 NEW: Example networks (use these!)
│   ├── README.md          # Example documentation
│   ├── lenet5.nlang       # LeNet-5 for MNIST
│   ├── vgg16.nlang        # VGG-16 for ImageNet
│   └── simple_classifier.nlang  # Minimal example
│
├── architecture/           # ⚠️ DEPRECATED: Old examples (to be removed)
│   ├── demo.nlang         # Use examples/lenet5.nlang instead
│   ├── mnist.nlang        # Use examples/lenet5.nlang instead
│   └── test_*.nlang       # Test cases (kept for regression tests)
│
├── models/                 # Pretrained weights (.nwf)
│   ├── README.md
│   ├── lenet5_mnist.nwf   # LeNet-5 trained on MNIST (~348 KB)
│   └── mnist_cnn.nwf      # Legacy variant (~1.6 MB)
│
├── tools/                  # Utilities
│   ├── README.md
│   ├── convert_pytorch.py
│   ├── convert_keras.py
│   ├── convert_onnx.py
│   ├── preprocess.py
│   ├── download_mnist_for_netlang.py
│   ├── test_network.c
│   ├── benchmark.c
│   └── create_test_weights.c
│
├── docs/                   # Documentation
│   ├── IMPLEMENTATION_GUIDE.md
│   └── WEIGHT_FORMAT.md
│
├── build/                  # Compiler build artifacts (gitignored)
│   ├── netlang.exe        # The NetLang compiler
│   └── *.o, *.c           # Build intermediates
│
├── generated/              # Generated C networks (gitignored)
├── bin/                    # Compiled executables (gitignored)
├── data/                   # Downloaded datasets (gitignored)
├── test_data/              # Test images (.bin format, gitignored)
│
├── README.md               # Project overview
├── QUICKSTART.md           # Getting started guide
├── CONTRIBUTING.md         # This file
├── Makefile                # Build system (Linux/macOS)
└── build.bat               # Build script (Windows)
```

### Path Migration (In Progress)

| Old Path | New Path | Status |
|----------|----------|--------|
| `architecture/mnist.nlang` | `examples/lenet5.nlang` | ✅ Migrated |
| `architecture/demo.nlang` | `examples/lenet5.nlang` | 🔄 To migrate |
| `architecture/vgg16.nlang` | `examples/vgg16.nlang` | ✅ Migrated |
| `architecture/inception.nlang` | `examples/inception.nlang` | 🔄 To migrate |
| `architecture/test_*.nlang` | Keep for regression tests | ⚠️ Keep |

---

## Development Setup

### Prerequisites

**Compiler Development:**
- GCC 6.0+ or Clang 10.0+
- Flex 2.6+
- Bison 3.0+
- Make (GNU Make 4.0+)
- CPU with AVX2 support (Intel Haswell 2013+)

**Tools Development:**
- Python 3.7+
- PyTorch / TensorFlow / ONNX (optional, for converters)

### Building the Compiler

**Linux/macOS:**
```bash
make            # Build compiler
make test       # Run test suite
make clean      # Clean artifacts
```

**Windows:**
```batch
:: Install MSYS2 from https://www.msys2.org/
:: Then in MSYS2 terminal:
pacman -S mingw-w64-x86_64-gcc flex bison make

:: Build:
build.bat
```

### Project Build System

**Makefile Targets:**
```bash
make             # Build compiler (default)
make test        # Run all tests
make clean       # Remove build artifacts
make install     # Install to system (future)
make docs        # Generate documentation (future)
```

**Build Output:**
- Compiler: `build/netlang.exe`
- Intermediate files: `build/*.o`, `build/*.c`, `build/*.h`
- Test results: `test_results/*.txt`

---

## Code Style

### C Code (Compiler)

**Style:**
- **Indentation:** 4 spaces (no tabs)
- **Braces:** K&R style (opening brace on same line)
- **Naming:**
  - Functions: `snake_case` (e.g., `parse_network`, `infer_shape`)
  - Types: `PascalCase` with typedef (e.g., `ASTNode`, `SymbolTable`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_LAYERS`, `TOKEN_EOF`)
- **Comments:** `//` for single-line, `/* */` for multi-line

**Example:**
```c
// Good
typedef struct {
    int type;
    char *name;
} LayerNode;

LayerNode* create_layer(int type, const char *name) {
    LayerNode *node = malloc(sizeof(LayerNode));
    node->type = type;
    node->name = strdup(name);
    return node;
}

// Bad
typedef struct{                    // Missing space before {
  int Type;                        // Wrong indentation (2 spaces)
  char* Name;                      // PascalCase for members (wrong)
}layer_node;                       // snake_case for type (wrong)

layer_node* CreateLayer(...)       // PascalCase for function (wrong)
```

### Python Code (Tools)

**Style:**
- **PEP 8 compliant**
- **Indentation:** 4 spaces
- **Line length:** 100 characters
- **Docstrings:** Google style
- **Type hints:** Use for function signatures

**Example:**
```python
# Good
def convert_pytorch_to_nwf(model_path: str, output_path: str) -> None:
    """Convert PyTorch model to NetLang weight format.
    
    Args:
        model_path: Path to .pth/.pt file
        output_path: Output .nwf file path
        
    Raises:
        FileNotFoundError: If model file doesn't exist
        ValueError: If model format is unsupported
    """
    pass

# Bad
def convert(model,output):         # No type hints
    # No docstring
    pass
```

### NetLang Code (.nlang files)

**Style:**
- **Indentation:** 4 spaces
- **Comments:** Document dimensions at each layer
- **Layer spacing:** Blank line between logical blocks
- **Naming:** CamelCase for networks/modules, snake_case for variables

**Example:**
```netlang
// Good
network LeNet5 {
    input(shape: [32, 32, 1])
    weights("models/lenet5_mnist.nwf")
    
    // Block 1: Feature extraction
    x = Conv2D(filters: 6, kernel: [5, 5], activation: relu) from input  // [28, 28, 6]
    x = MaxPool(pool: [2, 2], stride: 2) from x  // [14, 14, 6]
    
    // Block 2: Higher-level features
    x = Conv2D(filters: 16, kernel: [5, 5], activation: relu) from x  // [10, 10, 16]
    x = MaxPool(pool: [2, 2], stride: 2) from x  // [5, 5, 16]
    
    // Classifier
    x = Flatten() from x  // [400]
    x = Dense(units: 120, activation: relu) from x  // [120]
    x = Dense(units: 84, activation: relu) from x  // [84]
    x = Dense(units: 10, activation: softmax) from x  // [10]
}

// Bad
network MNIST{                     // No space before {
x=Conv2D(filters:6,kernel:[5,5],activation:relu) from input  // No spaces around =, :
x=MaxPool(pool:[2,2],stride:2)from x  // No space before 'from'
// No comments on dimensions
}
```

---

## Adding Features

### Add a New Layer Type

**Steps:**

1. **Lexer** - Add token in `src/lexer/net_lang.l`:
```c
"BatchNorm"     { return BATCHNORM; }
```

2. **Parser** - Add to `src/parser/net_lang.y`:
```c
%token BATCHNORM

layer_call:
    | BATCHNORM '(' layer_params ')' { $$ = create_layer_node(LAYER_BATCHNORM, $3); }
```

3. **AST** - Add enum in `src/ast/ast.h`:
```c
typedef enum {
    LAYER_CONV2D,
    LAYER_DENSE,
    LAYER_BATCHNORM,  // New
    ...
} LayerType;
```

4. **Shape Inference** - Add case in `src/semantic/type_checker.c`:
```c
Shape* infer_layer_shape(LayerNode *layer, Shape *input_shape) {
    switch (layer->type) {
        case LAYER_BATCHNORM:
            // BatchNorm preserves input shape
            return copy_shape(input_shape);
        ...
    }
}
```

5. **Code Generation** - Implement kernel in `src/codegen/kernels.c`:
```c
void batchnorm_forward(float *input, float *output, 
                       float *gamma, float *beta,
                       float *mean, float *var,
                       int size) {
    __m256 eps = _mm256_set1_ps(1e-5f);
    for (int i = 0; i < size; i += 8) {
        __m256 x = _mm256_loadu_ps(&input[i]);
        __m256 m = _mm256_loadu_ps(&mean[i % channels]);
        __m256 v = _mm256_loadu_ps(&var[i % channels]);
        __m256 g = _mm256_loadu_ps(&gamma[i % channels]);
        __m256 b = _mm256_loadu_ps(&beta[i % channels]);
        
        // (x - mean) / sqrt(var + eps) * gamma + beta
        __m256 norm = _mm256_div_ps(_mm256_sub_ps(x, m), 
                                     _mm256_sqrt_ps(_mm256_add_ps(v, eps)));
        __m256 out = _mm256_fmadd_ps(norm, g, b);
        _mm256_storeu_ps(&output[i], out);
    }
}
```

6. **Testing** - Add test in `examples/test_batchnorm.nlang`:
```netlang
network TestBatchNorm {
    input(shape: [32, 32, 3])
    x = Conv2D(filters: 64, kernel: [3,3]) from input
    x = BatchNorm() from x  // Test new layer
    x = Dense(units: 10, activation: softmax) from x
}
```

---

## Testing

### Compiler Tests

**Location:** `examples/test_*.nlang`

**Test Categories:**
1. **Valid Networks** - Should compile without errors
   - `test_minimal_valid.nlang` - Simplest valid network
   - `test_valid.nlang` - Complex valid network
   
2. **Invalid Networks** - Should fail with clear errors
   - `test_empty.nlang` - Empty network (should fail)
   - `test_errors.nlang` - Various semantic errors
   - `test_wrong_order.nlang` - Invalid dependency order

3. **Edge Cases**
   - `test_strict_order.nlang` - Declaration order enforcement
   - `test_only_input.nlang` - Network with only input (should fail)

**Running Tests:**
```bash
make test
# or
bash run_tests.bat  # Windows
```

### Testing Checklist

Before submitting a PR:

- [ ] Compiler builds without warnings
- [ ] All existing tests pass
- [ ] New features have corresponding tests
- [ ] Code follows style guide
- [ ] Documentation updated (README, examples/README.md, etc.)
- [ ] Commit messages are descriptive

---

## Pull Request Process

### Before Submitting

1. **Fork the repository**
2. **Create a feature branch:**
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make changes following style guidelines**
4. **Test thoroughly**
5. **Update documentation**
6. **Commit with clear messages:**
   ```bash
   git commit -m "Add BatchNorm layer support
   
   - Implement shape inference for BatchNorm
   - Add AVX2-optimized forward pass
   - Include test cases
   - Update examples/README.md"
   ```

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement

## Testing
How did you test this?
- [ ] Compiler tests pass
- [ ] Manual testing with example networks
- [ ] Performance benchmarks (if applicable)

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] No new warnings
- [ ] Tests added/updated
```

### Review Process

1. Automated checks run (build, tests)
2. Maintainer review (1-3 days)
3. Address feedback
4. Approval and merge

---

## Areas for Contribution

### High Priority

- [ ] **ARM NEON backend** - Mobile/embedded support
- [ ] **Additional layers** - Batch/LayerNorm, Dropout, Residual blocks
- [ ] **INT8 quantization** - Faster inference, smaller models
- [ ] **Model visualization** - GraphViz output for .nlang files
- [ ] **Error messages** - More helpful diagnostics
- [ ] **Documentation** - More examples, tutorials

### Medium Priority

- [ ] **LLVM IR backend** - Multi-platform support
- [ ] **Training support** - Backpropagation
- [ ] **Dynamic batch sizes** - Inference on multiple images
- [ ] **GPU backend** - CUDA/OpenCL support
- [ ] **Model compression** - Pruning, knowledge distillation

### Low Priority (Future Work)

- [ ] **Recurrent layers** - LSTM, GRU
- [ ] **Attention mechanisms** - Transformers
- [ ] **Object detection** - YOLO, SSD
- [ ] **Segmentation** - U-Net, Mask R-CNN

---

## Community

### Getting Help

- **Issues:** GitHub Issues for bugs/feature requests
- **Discussions:** GitHub Discussions for questions
- **Email:** [maintainer@example.com](mailto:maintainer@example.com)

### Code of Conduct

Be respectful, constructive, and professional. We're all here to learn and build something useful.

---

## License

By contributing, you agree your contributions will be licensed under the MIT License.

---

**Thank you for contributing to NetLang!** 🚀

*"Compile once, infer fast."*
