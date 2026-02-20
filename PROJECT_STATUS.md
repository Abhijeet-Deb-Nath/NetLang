# NetLang Project Organization

**Status:** ✅ Clean and organized (February 2026)

---

## Directory Structure

```
flex_work/
│
├── README.md                  # Main entry point
├── CONTRIBUTING.md            # Contribution guidelines
├── Makefile                   # Build system (Linux/macOS)
├── build.bat                  # Build script (Windows)
│
├── src/                       # Compiler source code (C)
│   ├── main.c                # Entry point
│   ├── lexer/                # Tokenization (Flex)
│   │   └── net_lang.l
│   ├── parser/               # Syntax analysis (Bison)
│   │   └── net_lang.y
│   ├── ast/                  # Abstract Syntax Tree
│   │   ├── ast.h
│   │   └── ast.c
│   ├── semantic/             # Type checking & shape inference
│   │   ├── semantic.h/c
│   │   ├── symbol_table.h/c
│   │   └── type_checker.h/c
│   └── codegen/              # Code generation
│       ├── codegen.h/c
│       ├── kernels.h/c
│       └── runtime.h/c
│
├── docs/                      # Documentation
│   ├── USER_GUIDE.md         # How to use NetLang
│   ├── ARCHITECTURE.md       # Compiler internals
│   ├── WEIGHT_FORMAT.md      # .nwf specification
│   ├── OPTIMIZATION_ROADMAP.md  # Performance plan
│   ├── IMPLEMENTATION_GUIDE.md  # Developer reference
│   ├── lexical_analysis.md   # Lexer details
│   └── parser_phase_analysis.md  # Parser details
│
├── architecture/              # Network definitions (.nlang)
│   ├── lenet5.nlang          # LeNet-5 architecture
│   ├── vgg16.nlang           # VGG-16 architecture
│   ├── inception.nlang       # Inception-style modules
│   ├── mnist.nlang           # Simple MNIST classifier
│   └── demo.nlang            # Demo network
│
├── models/                    # Trained weights (.nwf)
│   ├── lenet5_mnist.nwf      # LeNet-5 trained on MNIST
│   ├── lenet5_trained.nwf    # Alternative weights
│   └── README.md
│
├── netlang_weight_format/     # Additional weight files
│   ├── lenet5.nwf
│   ├── mnist_cnn.nwf
│   └── README.md
│
├── onnx_weight_files/         # ONNX source models
│   ├── lenet_mnist.onnx
│   └── README.md
│
├── examples/                  # Example architectures
│   ├── lenet5.nlang
│   ├── lenet_trained.nlang
│   ├── simple_classifier.nlang
│   ├── vgg16.nlang
│   └── README.md
│
├── tools/                     # Utilities
│   ├── convert_pytorch.py    # PyTorch → .nwf converter
│   ├── convert_onnx.py       # ONNX → .nwf converter
│   ├── convert_keras.py      # Keras → .nwf converter
│   ├── preprocess.py         # Image preprocessing
│   ├── download_mnist_for_netlang.py  # MNIST downloader
│   ├── analyze_onnx.py       # ONNX analysis tool
│   ├── check_nwf.py          # .nwf validation
│   ├── compare_inference.py  # Inference comparison
│   ├── test_network.c        # C test harness
│   ├── benchmark.c           # Performance benchmark
│   └── README.md
│
├── generated/                 # Generated C code (output)
│   ├── generated_lenet5.c
│   ├── generated_mnist.c
│   └── README.md
│
├── bin/                       # Compiled executables
│   └── README.md
│
├── build/                     # Compiler build artifacts
│   ├── lex.yy.c
│   ├── net_lang.tab.c
│   └── net_lang.tab.h
│
├── data/                      # Datasets
│   └── MNIST/
│       └── raw/
│
├── test_data/                 # Test images
│   ├── preprocessed/
│   └── preprocessed_28x28/
│
├── test_results/              # Test outputs
│
├── benchmark_frameworks.py    # Framework comparison script
└── test_accuracy.py           # Accuracy testing script

```

---

## File Purposes

### Root Level

| File | Purpose |
|------|---------|
| README.md | Main project documentation and quick start |
| CONTRIBUTING.md | Guidelines for contributors |
| Makefile | Build system for Linux/macOS |
| build.bat | Build script for Windows |
| benchmark_frameworks.py | Compare NetLang with production frameworks |
| test_accuracy.py | Validate accuracy on test sets |

### Documentation (docs/)

| File | Content |
|------|---------|
| USER_GUIDE.md | Complete usage guide with examples |
| ARCHITECTURE.md | Compiler design and implementation details |
| WEIGHT_FORMAT.md | .nwf binary format specification |
| OPTIMIZATION_ROADMAP.md | Performance improvement plan (4-phase) |
| IMPLEMENTATION_GUIDE.md | Developer reference for extending compiler |
| lexical_analysis.md | Lexer implementation details |
| parser_phase_analysis.md | Parser and AST construction |

### Source Code (src/)

| Component | Files | Purpose |
|-----------|-------|---------|
| Main | main.c | Entry point, argument parsing |
| Lexer | lexer/net_lang.l | Tokenization (Flex) |
| Parser | parser/net_lang.y | Syntax analysis (Bison) |
| AST | ast/*.{h,c} | Abstract syntax tree structures |
| Semantic | semantic/*.{h,c} | Type checking, shape inference |
| Codegen | codegen/*.{h,c} | C code generation with AVX2 |

### Utilities (tools/)

| Tool | Purpose |
|------|---------|
| convert_pytorch.py | Convert PyTorch models to .nwf |
| convert_onnx.py | Convert ONNX models to .nwf |
| convert_keras.py | Convert Keras models to .nwf |
| preprocess.py | Prepare images for inference |
| download_mnist_for_netlang.py | Download MNIST dataset |
| analyze_onnx.py | Inspect ONNX model structure |
| check_nwf.py | Validate .nwf files |
| compare_inference.py | Compare outputs with reference |
| test_network.c | C-based inference testing |
| benchmark.c | Performance measurement |

---

## Documentation Organization

### For Users (Getting Started)

1. **[README.md](../README.md)** - Start here
2. **[docs/USER_GUIDE.md](USER_GUIDE.md)** - Complete usage guide
3. **[examples/README.md](../examples/README.md)** - Example networks

### For Understanding the System

1. **[docs/ARCHITECTURE.md](ARCHITECTURE.md)** - System design
2. **[docs/WEIGHT_FORMAT.md](WEIGHT_FORMAT.md)** - Weight file format
3. **[docs/lexical_analysis.md](lexical_analysis.md)** - Lexer details
4. **[docs/parser_phase_analysis.md](parser_phase_analysis.md)** - Parser details

### For Developers (Contributing)

1. **[CONTRIBUTING.md](../CONTRIBUTING.md)** - Contribution guidelines
2. **[docs/IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md)** - Developer reference
3. **[docs/OPTIMIZATION_ROADMAP.md](OPTIMIZATION_ROADMAP.md)** - Future work

---

## Recent Cleanup (February 2026)

### Removed Files

**Redundant documentation:**
- ❌ README_NEW.md (merged into README.md)
- ❌ PROJECT_OVERVIEW.md (merged into ARCHITECTURE.md)
- ❌ PROJECT_ORGANIZATION.md (replaced by this file)
- ❌ QUICKSTART.md (merged into USER_GUIDE.md)
- ❌ INFERENCE_GUIDE.md (merged into USER_GUIDE.md)
- ❌ RESTRUCTURE_PLAN.md (completed, archived)
- ❌ RESTRUCTURE_SUMMARY.md (completed, archived)
- ❌ PERFORMANCE_SYSTEM_COMPLETE.md (merged into OPTIMIZATION_ROADMAP.md)

**Debug/temporary files:**
- ❌ debug.txt
- ❌ debug_nwf.py
- ❌ debug_write.py
- ❌ restructure_repo.ps1
- ❌ test_sizes.c
- ❌ test_sizes.exe

**Moved to proper locations:**
- ✅ analyze_onnx.py → tools/
- ✅ check_nwf.py → tools/
- ✅ compare_inference.py → tools/
- ✅ explanation/*.md → docs/

### Created Files

**New comprehensive documentation:**
- ✅ docs/ARCHITECTURE.md (18 KB) - Complete system design
- ✅ docs/USER_GUIDE.md (15 KB) - Complete usage guide
- ✅ docs/OPTIMIZATION_ROADMAP.md (18 KB) - Performance improvement plan

---

## Key Workflows

### Compile and Run

```bash
# 1. Build compiler
build.bat  # or make

# 2. Compile network
bin/netlang_compiler architecture/lenet5.nlang -o generated/lenet5.c

# 3. Build executable
gcc -O3 -mavx2 generated/lenet5.c -o lenet5_infer -lm

# 4. Run inference
./lenet5_infer input.bin
```

### Convert and Test

```bash
# Convert PyTorch model
python tools/convert_pytorch.py model.pth models/model.nwf

# Test accuracy
python test_accuracy.py --network architecture/lenet5.nlang --weights models/lenet5_mnist.nwf

# Benchmark performance
python benchmark_frameworks.py
```

---

## Documentation Quality Standards

All documentation follows these principles:

1. **Concise** - No unnecessary verbosity
2. **Structured** - Clear sections with headers
3. **Practical** - Includes working examples
4. **Complete** - Covers all features
5. **Cross-referenced** - Links to related docs
6. **Up-to-date** - Reflects current implementation

---

## Next Steps

See [docs/OPTIMIZATION_ROADMAP.md](OPTIMIZATION_ROADMAP.md) for the performance improvement plan:

**Phase 1 (3 months):**
1. Operator fusion (2.5x speedup)
2. Cache blocking (3x speedup)
3. Multi-threading (2x speedup)
4. FMA instructions (1.3x speedup)

**Target:** 10-15x overall improvement (10ms → 0.6-1ms)

---

*Last updated: February 20, 2026*
