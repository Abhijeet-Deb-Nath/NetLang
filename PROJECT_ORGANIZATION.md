# Project Organization Guide

This document describes the organized structure of the NetLang Compiler repository.

## Directory Structure

```
flex_work/
│
├── bin/                          # Generated executables
│   ├── README.md                 # Documentation for this folder
│   ├── .gitkeep                  # Preserve folder in git
│   ├── mnist.exe                 # Compiled network inference programs
│   └── *.exe                     # Other generated executables
│
├── build/                        # Compiler build artifacts
│   ├── netlang.exe               # NetLang compiler (main tool)
│   ├── *.o                       # Object files
│   ├── lex.yy.c                  # Generated lexer
│   └── net_lang.tab.*            # Generated parser
│
├── docs/                         # Documentation
│   ├── IMPLEMENTATION_GUIDE.md   # Code generation guide
│   └── WEIGHT_FORMAT.md          # .nwf format specification
│
├── examples/                     # Example network definitions
│   ├── mnist.nlang               # LeNet-5 for MNIST
│   ├── vgg16.nlang               # VGG-16 architecture
│   ├── inception.nlang           # Inception modules
│   └── *.nlang                   # Test cases
│
├── explanation/                  # Design documentation
│   ├── lexical_analysis.md       # Lexer phase details
│   └── parser_phase_analysis.md  # Parser phase details
│
├── generated/                    # Generated C source code
│   ├── README.md                 # Documentation for this folder
│   ├── .gitkeep                  # Preserve folder in git
│   └── *.c                       # Auto-generated inference code
│
├── models/                       # Weight files (.nwf format)
│   ├── README.md                 # Documentation for this folder
│   ├── .gitkeep                  # Preserve folder in git
│   ├── lenet5.nwf                # LeNet-5 weights
│   ├── mnist_cnn.nwf             # MNIST CNN weights
│   └── *.nwf                     # Other converted weights
│
├── onnx_weight_files/            # Original ONNX models
│   ├── README.md                 # Documentation for this folder
│   ├── .gitkeep                  # Preserve folder in git
│   ├── lenet_mnist.onnx          # LeNet-5 ONNX model
│   └── *.onnx                    # Other ONNX models
│
├── src/                          # Compiler source code
│   ├── main.c                    # Compiler entry point
│   ├── ast/                      # Abstract Syntax Tree
│   ├── codegen/                  # Code generation & runtime
│   │   ├── codegen.c             # C code generator
│   │   ├── runtime.c             # Weight loading (mmap)
│   │   ├── kernels.c             # AVX2 inference kernels
│   │   └── *.h                   # Headers
│   ├── lexer/                    # Lexical analysis
│   │   └── net_lang.l            # Flex specification
│   ├── parser/                   # Syntax analysis
│   │   └── net_lang.y            # Bison grammar
│   └── semantic/                 # Semantic analysis
│       ├── semantic.c            # Semantic analyzer
│       ├── symbol_table.c        # Symbol table
│       └── type_checker.c        # Type & shape inference
│
├── test_results/                 # Test outputs
│   └── *.txt                     # Validation results
│
├── tools/                        # Utility scripts
│   ├── convert_pytorch.py        # PyTorch → .nwf
│   ├── convert_onnx.py           # ONNX → .nwf
│   ├── convert_keras.py          # Keras → .nwf
│   ├── convert_lenet_weights.py  # LeNet-specific converter
│   ├── benchmark.c               # Performance testing
│   └── create_test_weights.c     # Test weight generator
│
├── .gitignore                    # Git ignore rules
├── build.bat                     # Windows build script
├── Makefile                      # Unix build system
├── PERFORMANCE_SYSTEM_COMPLETE.md # Performance documentation
├── QUICKSTART.md                 # Quick start guide
├── README.md                     # Main documentation
└── run_tests.bat                 # Test runner script
```

## Workflow

### 1. **Obtain Weights** → `onnx_weight_files/` or Train Your Own

```bash
# Download/export ONNX model
python -c "torch.onnx.export(...)"  # → onnx_weight_files/model.onnx

# Or train and save PyTorch model
python train.py  # → model.pth
```

### 2. **Convert to .nwf** → `models/`

```bash
# From ONNX
python tools/convert_onnx.py \
    --model onnx_weight_files/lenet_mnist.onnx \
    --output models/lenet5.nwf

# From PyTorch
python tools/convert_pytorch.py \
    --model model.pth \
    --output models/model.nwf
```

### 3. **Write Network Definition** → `examples/`

```netlang
// examples/mnist.nlang
network MNIST_CNN {
    input(shape: [32, 32, 1])
    weights("models/lenet5.nwf")  // Reference converted weights
    
    x = Conv2D(filters: 6, kernel: [5,5], activation: relu) from input
    // ... network definition
}
```

### 4. **Compile Network** → `generated/`

```bash
# Generate C code
build\netlang.exe examples\mnist.nlang -o generated\mnist.c
```

### 5. **Build Executable** → `bin/`

```bash
# Compile to optimized executable
gcc -O3 -march=haswell -mavx2 -mfma ^
    generated\mnist.c ^
    src\codegen\runtime.c ^
    src\codegen\kernels.c ^
    -o bin\mnist.exe
```

### 6. **Run Inference**

```bash
bin\mnist.exe test_image.png
```

## Key Files by Purpose

### For Users
- `README.md` - Start here
- `QUICKSTART.md` - Get started in 5 minutes
- `examples/*.nlang` - Example networks
- `tools/convert_*.py` - Weight converters

### For Developers
- `src/` - Compiler implementation
- `docs/IMPLEMENTATION_GUIDE.md` - Code generation details
- `docs/WEIGHT_FORMAT.md` - .nwf specification
- `explanation/` - Phase-by-phase design docs

### Generated/Build Artifacts (Git Ignored)
- `bin/*.exe` - Network executables
- `generated/*.c` - Generated inference code
- `build/*.o` - Object files

### Assets (Can be Git LFS)
- `models/*.nwf` - Converted weights (3MB - 550MB)
- `onnx_weight_files/*.onnx` - Original models

## Git Ignore Strategy

The `.gitignore` file excludes:
- Build artifacts (`*.o`, `*.exe` in `build/`)
- Generated code (`generated/*.c`)
- Generated executables (`bin/*.exe`)
- IDE files (`.vscode/`, `.vs/`)
- Python cache (`__pycache__/`)

But **keeps**:
- Source code (`src/`)
- Examples (`examples/`)
- Documentation (`docs/`, `README.md`)
- Tools (`tools/`)
- `.gitkeep` files to preserve folder structure

**Optional**: You may want to ignore large weight files:
```gitignore
models/*.nwf
onnx_weight_files/*.onnx
```

Use Git LFS for large model files if tracking them in version control.

## Cleaning Up

### Clean Build Artifacts
```bash
make clean
# Or manually:
rm build/*.o build/*.exe
```

### Clean Generated Files
```bash
rm generated/*.c
rm bin/*.exe
```

### Full Clean
```bash
make clean
rm generated/*.c bin/*.exe
# Weight files are kept intentionally
```

## Benefits of This Organization

1. **Clear Separation of Concerns**
   - Source code vs. generated code
   - Original models vs. converted weights
   - Build artifacts vs. runtime executables

2. **Easy to Clean**
   - Delete `bin/` and `generated/` without affecting source
   - Rebuild from scratch quickly

3. **Git-Friendly**
   - Ignored files won't clutter commits
   - `.gitkeep` preserves empty folders
   - Optional LFS for large models

4. **Discoverable**
   - Each folder has a README.md
   - Clear naming conventions
   - Logical workflow progression

5. **Scalable**
   - Easy to add more models
   - Multiple networks can coexist
   - No naming conflicts

## Migration from Old Structure

If you have files from the old structure:

```bash
# Move executables
mv *.exe bin/

# Move generated C files
mv generated_*.c generated/

# Weight files already in models/ - OK!
# ONNX files already in onnx_weight_files/ - OK!
```

---

**Maintained by:** Abhijeet Deb Nath  
**Last Updated:** February 20, 2026
