# Repository Restructuring Plan

**Goal:** Transform the current scattered structure into a clean, professional, well-documented repository.

## Current Issues

1. **Documentation scattered** across root (5+ markdown files)
2. **Redundant directories**: `architecture/` + `examples/`, `netlang_weight_format/` + `models/`
3. **Build artifacts mixed** with source code
4. **Unclear naming**: `explanation/` vs `docs/`, `onnx_weight_files/` purpose unclear
5. **Test infrastructure** needs organization

---

## Proposed Clean Structure

```
netlang/
├── README.md                      # Main project overview & quickstart
├── LICENSE                        # MIT/Apache license
├── .gitignore                     # Comprehensive ignore rules
├── Makefile                       # Primary build system
│
├── docs/                          # 📚 All documentation in one place
│   ├── README.md                 # Documentation index
│   ├── quickstart.md             # 5-minute getting started
│   ├── language-reference.md     # NetLang syntax & semantics
│   ├── compiler-architecture.md  # Compiler design (phases 1-5)
│   ├── weight-format-spec.md     # .nwf binary format specification
│   ├── inference-guide.md        # Using generated C code
│   ├── performance-analysis.md   # Benchmarks & optimization notes
│   └── implementation-notes.md   # Development history & design decisions
│
├── src/                           # 🔧 Compiler source code (C)
│   ├── main.c                    # Entry point
│   ├── lexer/
│   │   └── net_lang.l           # Flex lexer
│   ├── parser/
│   │   └── net_lang.y           # Bison parser
│   ├── ast/
│   │   ├── ast.h
│   │   └── ast.c                # AST node definitions
│   ├── semantic/
│   │   ├── semantic.h/c         # Semantic analyzer
│   │   ├── symbol_table.h/c     # Symbol table
│   │   └── type_checker.h/c     # Type checking & shape inference
│   └── codegen/
│       ├── codegen.h/c           # C code generator
│       ├── runtime.h/c           # Runtime library (weight loading)
│       └── kernels.h/c           # Optimized Conv2D/Dense/Pool kernels
│
├── examples/                      # 📝 Example .nlang networks
│   ├── README.md                 # Examples guide
│   ├── mnist_lenet5.nlang        # MNIST digit classification
│   ├── vgg16_imagenet.nlang      # VGG-16 for ImageNet
│   ├── inception_module.nlang    # Inception block demo
│   └── simple_classifier.nlang   # Minimal example
│
├── tools/                         # ⚙️ Utilities & converters
│   ├── README.md                 # Tools documentation
│   ├── weight_converters/
│   │   ├── convert_pytorch.py   # PyTorch → .nwf
│   │   ├── convert_keras.py     # Keras → .nwf
│   │   └── convert_onnx.py      # ONNX → .nwf
│   ├── preprocessing/
│   │   ├── preprocess_image.py  # Single image → .bin
│   │   └── download_mnist.py    # MNIST dataset → .bin batch
│   ├── testing/
│   │   ├── test_network.c       # C inference test harness
│   │   └── benchmark.c          # Performance benchmarking tool
│   └── create_test_weights.c     # Generate dummy .nwf files
│
├── tests/                         # 🧪 Test suite
│   ├── unit/                     # Unit tests (future)
│   ├── integration/              # End-to-end tests
│   └── fixtures/                 # Test .nlang files
│       ├── valid/                # Should compile successfully
│       │   ├── simple_dense.nlang
│       │   ├── conv_network.nlang
│       │   └── inception_block.nlang
│       └── invalid/              # Should fail with errors
│           ├── undefined_var.nlang
│           ├── shape_mismatch.nlang
│           └── circular_dep.nlang
│
├── models/                        # 🎯 Pretrained weights (.nwf files)
│   ├── README.md                 # Weight file descriptions
│   ├── lenet5_mnist.nwf         # LeNet-5 trained on MNIST (32×32)
│   └── .gitignore               # Ignore large weight files
│
├── scripts/                       # 🚀 Build & automation scripts
│   ├── build.sh                  # Linux/macOS build script
│   ├── build.bat                 # Windows build script
│   ├── test.sh                   # Run all tests
│   └── clean.sh                  # Clean build artifacts
│
└── [.gitignore these]
    ├── build/                    # Build artifacts (compiler output)
    │   ├── intermediate/        # .o files, .tab.c, lex.yy.c
    │   ├── compiler/            # netlang executable
    │   └── generated/           # Generated C network code
    ├── data/                     # Downloaded datasets (MNIST, etc.)
    └── test_output/             # Test execution results
```

---

## Migration Steps

### Step 1: Create New Directory Structure
```powershell
# Create documentation directory
mkdir docs_new

# Create examples directory  
mkdir examples_new

# Create tools subdirectories
mkdir tools\weight_converters
mkdir tools\preprocessing
mkdir tools\testing

# Create tests structure
mkdir tests\unit
mkdir tests\integration
mkdir tests\fixtures\valid
mkdir tests\fixtures\invalid

# Create scripts directory
mkdir scripts
```

### Step 2: Move & Consolidate Documentation
```powershell
# Consolidate all docs
Move-Item QUICKSTART.md docs_new\quickstart.md
Move-Item INFERENCE_GUIDE.md docs_new\inference-guide.md
Move-Item PERFORMANCE_SYSTEM_COMPLETE.md docs_new\performance-analysis.md
Move-Item docs\IMPLEMENTATION_GUIDE.md docs_new\implementation-notes.md
Move-Item docs\WEIGHT_FORMAT.md docs_new\weight-format-spec.md
Move-Item explanation\lexical_analysis.md docs_new\compiler-architecture.md
Move-Item PROJECT_ORGANIZATION.md docs_new\legacy-organization.md

# Remove old doc directories
Remove-Item explanation -Recurse
Remove-Item docs -Recurse

# Rename new docs directory
Rename-Item docs_new docs
```

### Step 3: Reorganize Examples
```powershell
# Move architecture files to examples
Copy-Item architecture\mnist.nlang examples_new\mnist_lenet5.nlang
Copy-Item architecture\vgg16.nlang examples_new\vgg16_imagenet.nlang
Copy-Item architecture\inception.nlang examples_new\inception_module.nlang
Copy-Item architecture\demo.nlang examples_new\simple_classifier.nlang

# Move test fixtures
Move-Item architecture\test_valid.nlang tests\fixtures\valid\
Move-Item architecture\test_errors.nlang tests\fixtures\invalid\
Move-Item architecture\test_*.nlang tests\fixtures\invalid\

# Remove old architecture directory
Remove-Item architecture -Recurse

# Rename examples
Rename-Item examples_new examples
```

### Step 4: Reorganize Tools
```powershell
# Weight converters
Move-Item tools\convert_pytorch.py tools\weight_converters\
Move-Item tools\convert_keras.py tools\weight_converters\
Move-Item tools\convert_onnx.py tools\weight_converters\
Move-Item tools\convert_lenet_weights.py tools\weight_converters\

# Preprocessing
Move-Item tools\preprocess.py tools\preprocessing\preprocess_image.py
Move-Item tools\download_mnist_for_netlang.py tools\preprocessing\download_mnist.py

# Testing
Move-Item tools\test_network.c tools\testing\
Move-Item tools\benchmark.c tools\testing\
```

### Step 5: Consolidate Models
```powershell
# Keep only models/ directory, remove redundant netlang_weight_format/
Remove-Item netlang_weight_format -Recurse

# Remove onnx_weight_files (move onnx to models if needed)
Move-Item onnx_weight_files\lenet_mnist.onnx models\
Remove-Item onnx_weight_files -Recurse
```

### Step 6: Reorganize Build Files
```powershell
# Move build scripts to scripts/
Move-Item build.bat scripts\
Move-Item build_test_harness.bat scripts\
Move-Item run_tests.bat scripts\test.bat

# Keep build/, generated/, bin/ as-is but update .gitignore
```

### Step 7: Update .gitignore
```gitignore
# Compiler build artifacts
build/
bin/
generated/

# Downloaded datasets
data/
test_data/

# Test outputs
test_output/
test_results/

# Large weight files (optional - commit small ones)
models/*.nwf
!models/lenet5_mnist.nwf

# Compiled files
*.exe
*.out
*.o
*.a
*.so
*.dll
lex.yy.c
*.tab.c
*.tab.h
*.output

# Python
__pycache__/
*.pyc
*.pyo

# Editor
.vscode/
.idea/
*.swp
*~
```

### Step 8: Create README files

**docs/README.md**
```markdown
# Documentation

- [Quickstart Guide](quickstart.md) - Get started in 5 minutes
- [Language Reference](language-reference.md) - NetLang syntax
- [Compiler Architecture](compiler-architecture.md) - How it works
- [Weight Format Spec](weight-format-spec.md) - .nwf file format
- [Inference Guide](inference-guide.md) - Using generated code
- [Performance Analysis](performance-analysis.md) - Benchmarks
```

**examples/README.md**
```markdown
# NetLang Examples

## Basic Examples
- `simple_classifier.nlang` - Minimal network (Dense layers only)

## Computer Vision
- `mnist_lenet5.nlang` - MNIST digit classification (32×32)
- `vgg16_imagenet.nlang` - VGG-16 for ImageNet classification
- `inception_module.nlang` - Inception block demonstration

## Usage
```bash
# Compile a network
./build/compiler/netlang examples/mnist_lenet5.nlang -o build/generated/mnist.c

# Generate executable
gcc -O3 -mavx2 -mfma build/generated/mnist.c -o build/bin/mnist
```
```

**tools/README.md**
```markdown
# NetLang Tools

## Weight Converters
Convert pretrained weights to .nwf format:
- `weight_converters/convert_pytorch.py` - PyTorch models
- `weight_converters/convert_keras.py` - Keras/TensorFlow models
- `weight_converters/convert_onnx.py` - ONNX models

## Preprocessing
Prepare input data:
- `preprocessing/preprocess_image.py` - Convert images to .bin format
- `preprocessing/download_mnist.py` - Download MNIST dataset

## Testing
Test and benchmark networks:
- `testing/test_network.c` - C test harness for inference
- `testing/benchmark.c` - Performance benchmarking
```

**models/README.md**
```markdown
# Pretrained Model Weights

## Available Models
- `lenet5_mnist.nwf` - LeNet-5 trained on MNIST (32×32 input, 10 classes)
  - Accuracy: ~98.5% on MNIST test set
  - Size: 348 KB
  - Architecture: Conv(6)→Pool→Conv(16)→Pool→Dense(120)→Dense(84)→Dense(10)

## Using Your Own Models
Convert from popular frameworks:

```bash
# PyTorch
python tools/weight_converters/convert_pytorch.py --model model.pth --output models/custom.nwf

# Keras
python tools/weight_converters/convert_keras.py --model model.h5 --output models/custom.nwf

# ONNX
python tools/weight_converters/convert_onnx.py --model model.onnx --output models/custom.nwf
```
```

---

## Path Updates Required

After restructuring, update these file references:

### 1. Main README.md
- Update all doc links to `docs/`
- Update example paths to `examples/`

### 2. Makefile
- Update `SRC` paths if needed
- Update `BUILD_DIR` variables

### 3. scripts/build.bat
- Adjust paths to `src/`, `build/`

### 4. examples/*.nlang
- Update `weights("models/...")` paths

### 5. tools/ scripts
- Update relative paths to `models/`, `data/`, `build/`

---

## Verification Checklist

After restructuring, verify:
- [ ] `make clean && make` builds successfully
- [ ] `scripts/build.bat` works on Windows
- [ ] Example networks compile: `netlang examples/mnist_lenet5.nlang`
- [ ] Weight converters run: `python tools/weight_converters/convert_pytorch.py --help`
- [ ] All documentation links work
- [ ] `.gitignore` properly excludes build artifacts
- [ ] README.md accurately reflects new structure

---

## Benefits of New Structure

✅ **Clear separation of concerns**: src/, examples/, tools/, docs/
✅ **Professional layout**: Matches Go, Rust, Python project standards
✅ **Easy navigation**: Everything has a logical place
✅ **Clean root**: Only essential files at top level
✅ **Better documentation**: Consolidated in docs/ with clear index
✅ **Scalable**: Easy to add new examples, tools, tests
✅ **CI/CD ready**: Clean build/ and scripts/ for automation

---

## Next Steps

1. **Review this plan** - Ensure it meets your needs
2. **Backup current state** - `git commit` or copy to safe location
3. **Execute migration** - Run PowerShell commands above
4. **Update path references** - Fix imports and links
5. **Test thoroughly** - Build and run examples
6. **Update README.md** - Reflect new structure
7. **Commit changes** - `git add . && git commit -m "Restructure repository"`
