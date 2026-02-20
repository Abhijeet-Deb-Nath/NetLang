# NetLang Repository Restructuring Script
# Run this script to reorganize the repository into a clean structure
# BACKUP YOUR FILES BEFORE RUNNING THIS!

Write-Host "NetLang Repository Restructuring" -ForegroundColor Cyan
Write-Host "=================================" -ForegroundColor Cyan
Write-Host ""

# Confirm before proceeding
$confirm = Read-Host "This will restructure your repository. Have you backed up? (yes/no)"
if ($confirm -ne "yes") {
    Write-Host "Aborting. Please backup first." -ForegroundColor Yellow
    exit
}

Write-Host "`n[1/8] Creating new directory structure..." -ForegroundColor Green

# Create new directories
$dirs = @(
    "docs",
    "examples",
    "tools\weight_converters",
    "tools\preprocessing",
    "tools\testing",
    "tests\unit",
    "tests\integration",
    "tests\fixtures\valid",
    "tests\fixtures\invalid",
    "scripts"
)

foreach ($dir in $dirs) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
        Write-Host "  Created: $dir"
    }
}

Write-Host "`n[2/8] Consolidating documentation..." -ForegroundColor Green

# Move and rename documentation files
$docMoves = @{
    "QUICKSTART.md" = "docs\quickstart.md"
    "INFERENCE_GUIDE.md" = "docs\inference-guide.md"
    "PERFORMANCE_SYSTEM_COMPLETE.md" = "docs\performance-analysis.md"
    "PROJECT_ORGANIZATION.md" = "docs\legacy-organization.md"
}

foreach ($src in $docMoves.Keys) {
    if (Test-Path $src) {
        Move-Item $src $docMoves[$src] -Force
        Write-Host "  Moved: $src -> $($docMoves[$src])"
    }
}

# Move docs from old docs/ folder
if (Test-Path "docs\IMPLEMENTATION_GUIDE.md") {
    Move-Item "docs\IMPLEMENTATION_GUIDE.md" "docs\implementation-notes.md" -Force -ErrorAction SilentlyContinue
}
if (Test-Path "docs\WEIGHT_FORMAT.md") {
    Move-Item "docs\WEIGHT_FORMAT.md" "docs\weight-format-spec.md" -Force -ErrorAction SilentlyContinue
}

# Move explanation files
if (Test-Path "explanation\lexical_analysis.md") {
    Copy-Item "explanation\lexical_analysis.md" "docs\lexical-analysis.md" -Force
}
if (Test-Path "explanation\parser_phase_analysis.md") {
    Copy-Item "explanation\parser_phase_analysis.md" "docs\parser-analysis.md" -Force
}

Write-Host "`n[3/8] Reorganizing examples..." -ForegroundColor Green

# Move architecture files to examples with better names
$exampleMoves = @{
    "architecture\mnist.nlang" = "examples\mnist_lenet5.nlang"
    "architecture\vgg16.nlang" = "examples\vgg16_imagenet.nlang"
    "architecture\inception.nlang" = "examples\inception_module.nlang"
    "architecture\demo.nlang" = "examples\simple_classifier.nlang"
}

foreach ($src in $exampleMoves.Keys) {
    if (Test-Path $src) {
        Copy-Item $src $exampleMoves[$src] -Force
        Write-Host "  Copied: $src -> $($exampleMoves[$src])"
    }
}

# Move test fixtures
if (Test-Path "architecture") {
    Get-ChildItem "architecture\test_*.nlang" | ForEach-Object {
        if ($_.Name -match "test_(valid|minimal)") {
            Copy-Item $_.FullName "tests\fixtures\valid\" -Force
        } else {
            Copy-Item $_.FullName "tests\fixtures\invalid\" -Force
        }
        Write-Host "  Moved test: $($_.Name)"
    }
}

Write-Host "`n[4/8] Reorganizing tools..." -ForegroundColor Green

# Weight converters
$weightConverters = @(
    "convert_pytorch.py",
    "convert_keras.py",
    "convert_onnx.py",
    "convert_lenet_weights.py"
)

foreach ($tool in $weightConverters) {
    $path = "tools\$tool"
    if (Test-Path $path) {
        Move-Item $path "tools\weight_converters\" -Force
        Write-Host "  Moved: $tool -> weight_converters\"
    }
}

# Preprocessing tools
if (Test-Path "tools\preprocess.py") {
    Move-Item "tools\preprocess.py" "tools\preprocessing\preprocess_image.py" -Force
    Write-Host "  Moved: preprocess.py -> preprocessing\preprocess_image.py"
}
if (Test-Path "tools\download_mnist_for_netlang.py") {
    Move-Item "tools\download_mnist_for_netlang.py" "tools\preprocessing\download_mnist.py" -Force
    Write-Host "  Moved: download_mnist_for_netlang.py -> preprocessing\download_mnist.py"
}

# Testing tools
$testTools = @("test_network.c", "benchmark.c", "create_test_weights.c")
foreach ($tool in $testTools) {
    $path = "tools\$tool"
    if (Test-Path $path) {
        Move-Item $path "tools\testing\" -Force
        Write-Host "  Moved: $tool -> testing\"
    }
}

Write-Host "`n[5/8] Consolidating model weights..." -ForegroundColor Green

# Remove redundant netlang_weight_format directory
if (Test-Path "netlang_weight_format") {
    Write-Host "  Removing redundant netlang_weight_format/"
    Remove-Item "netlang_weight_format" -Recurse -Force -ErrorAction SilentlyContinue
}

# Move ONNX file to models
if (Test-Path "onnx_weight_files\lenet_mnist.onnx") {
    Copy-Item "onnx_weight_files\lenet_mnist.onnx" "models\" -Force
    Write-Host "  Moved: lenet_mnist.onnx -> models/"
}

# Remove onnx_weight_files directory
if (Test-Path "onnx_weight_files") {
    Remove-Item "onnx_weight_files" -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host "`n[6/8] Reorganizing build scripts..." -ForegroundColor Green

# Move build scripts
$buildScripts = @("build.bat", "build_test_harness.bat", "run_tests.bat")
foreach ($script in $buildScripts) {
    if (Test-Path $script) {
        Move-Item $script "scripts\" -Force
        Write-Host "  Moved: $script -> scripts\"
    }
}

# Rename run_tests.bat to test.bat
if (Test-Path "scripts\run_tests.bat") {
    Rename-Item "scripts\run_tests.bat" "test.bat" -Force
}

Write-Host "`n[7/8] Updating .gitignore..." -ForegroundColor Green

$gitignoreContent = @"
# Compiler build artifacts
build/
bin/
generated/

# Downloaded datasets (large files)
data/
test_data/

# Test outputs
test_output/
test_results/

# Large weight files (commit small ones)
models/*.nwf
models/*.onnx
!models/lenet5_mnist.nwf

# Compiled files
*.exe
*.out
*.o
*.a
*.so
*.dll

# Generated parser/lexer files
lex.yy.c
*.tab.c
*.tab.h
*.output

# Python
__pycache__/
*.pyc
*.pyo
.pytest_cache/
venv/
.env

# IDEs and editors
.vscode/
.idea/
*.swp
*~
.DS_Store

# OS
Thumbs.db
desktop.ini

# Backup files
*.bak
*~
"@

Set-Content -Path ".gitignore" -Value $gitignoreContent
Write-Host "  Updated .gitignore"

Write-Host "`n[8/8] Creating README files..." -ForegroundColor Green

# docs/README.md
$docsReadme = @"
# NetLang Documentation

## Quick Links

- **[Quickstart Guide](quickstart.md)** - Get started in 5 minutes
- **[Language Reference](language-reference.md)** - NetLang syntax & semantics
- **[Compiler Architecture](compiler-architecture.md)** - How the compiler works
- **[Weight Format Specification](weight-format-spec.md)** - .nwf binary format
- **[Inference Guide](inference-guide.md)** - Using generated C code
- **[Performance Analysis](performance-analysis.md)** - Benchmarks & optimizations

## Development Notes

- **[Implementation Notes](implementation-notes.md)** - Design decisions
- **[Lexical Analysis](lexical-analysis.md)** - Tokenizer implementation
- **[Parser Analysis](parser-analysis.md)** - Grammar and AST

## Legacy Documentation

- [Legacy Organization](legacy-organization.md) - Original structure (archived)
"@
Set-Content -Path "docs\README.md" -Value $docsReadme

# examples/README.md
$examplesReadme = @"
# NetLang Examples

## Available Examples

### Basic
- **simple_classifier.nlang** - Minimal network (Dense layers only)
  - Good starting point for learning NetLang syntax

### Computer Vision
- **mnist_lenet5.nlang** - MNIST digit classification (LeNet-5)
  - Input: 32×32 grayscale images
  - Output: 10 classes (digits 0-9)
  - Architecture: Conv→Pool→Conv→Pool→Dense→Dense→Softmax

- **vgg16_imagenet.nlang** - VGG-16 for ImageNet classification
  - Input: 224×224 RGB images  
  - Output: 1000 classes
  - Deep architecture with 16 weight layers

- **inception_module.nlang** - Inception block demonstration
  - Shows module system and parallel branches

## Usage

### 1. Compile a Network
``````bash
# Build the compiler first
make

# Compile a .nlang file to C code
./build/netlang examples/mnist_lenet5.nlang -o build/generated/mnist.c
``````

### 2. Build the Executable
``````bash
gcc -O3 -march=haswell -mavx2 -mfma \
    build/generated/mnist.c \
    src/codegen/runtime.c \
    src/codegen/kernels.c \
    -o build/bin/mnist
``````

### 3. Run Inference
``````bash
# Preprocess an image
python tools/preprocessing/preprocess_image.py input.png input.bin

# Run inference
./build/bin/mnist input.bin
``````

## Creating Your Own Networks

See [docs/language-reference.md](../docs/language-reference.md) for full syntax.

Basic template:
``````netlang
network MyNetwork {
    input(shape: [H, W, C])
    weights("models/my_weights.nwf")
    
    x = Conv2D(filters: 32, kernel: [3, 3], activation: relu) from input
    x = MaxPool(pool: [2, 2], stride: 2) from x
    x = Flatten() from x
    x = Dense(units: 10, activation: softmax) from x
}
``````
"@
Set-Content -Path "examples\README.md" -Value $examplesReadme

# tools/README.md
$toolsReadme = @"
# NetLang Tools

Utilities for working with NetLang: weight conversion, data preprocessing, and testing.

## Weight Converters

Convert pretrained weights from popular frameworks to NetLang's .nwf format:

### PyTorch Conversion
``````bash
python tools/weight_converters/convert_pytorch.py \
    --model model.pth \
    --architecture lenet5 \
    --output models/my_model.nwf
``````

### Keras/TensorFlow Conversion
``````bash
python tools/weight_converters/convert_keras.py \
    --model model.h5 \
    --output models/my_model.nwf
``````

### ONNX Conversion
``````bash
python tools/weight_converters/convert_onnx.py \
    --model model.onnx \
    --output models/my_model.nwf
``````

## Preprocessing

### Single Image Preprocessing
``````bash
python tools/preprocessing/preprocess_image.py \
    input.jpg \
    output.bin \
    --size 32 32
``````

Converts images to raw float32 binary format (.bin) for inference.

### MNIST Dataset Download
``````bash
python tools/preprocessing/download_mnist.py
``````

Downloads MNIST test set and converts all 10,000 images to .bin format.
Output: `test_data/preprocessed/mnist_XXXX_label_Y.bin`

## Testing & Benchmarking

### C Test Harness
``````c
// Compile: gcc tools/testing/test_network.c generated_network.c -o test
// Run: ./test input.bin

// Simple test harness for running inference on .bin files
``````

### Performance Benchmarking
``````bash
gcc -O3 -mavx2 tools/testing/benchmark.c network.c -o bench
./bench input.bin --iterations 1000
``````

Measures inference latency and throughput.

## Directory Structure

``````
tools/
├── weight_converters/       # Framework → .nwf conversion
│   ├── convert_pytorch.py
│   ├── convert_keras.py
│   └── convert_onnx.py
├── preprocessing/           # Data preprocessing
│   ├── preprocess_image.py
│   └── download_mnist.py
└── testing/                # Testing utilities
    ├── test_network.c
    ├── benchmark.c
    └── create_test_weights.c
``````
"@
Set-Content -Path "tools\README.md" -Value $toolsReadme

# models/README.md
$modelsReadme = @"
# Model Weights

Pretrained neural network weights in NetLang's `.nwf` (NetLang Weight Format) binary format.

## Available Models

### lenet5_mnist.nwf
- **Architecture:** LeNet-5
- **Dataset:** MNIST (handwritten digits)
- **Input:** 32×32 grayscale (1 channel)
- **Output:** 10 classes (digits 0-9)
- **Size:** ~348 KB
- **Accuracy:** ~98.5% on MNIST test set

**Layers:**
- Conv2D(6, 5×5) → ReLU → MaxPool(2×2)
- Conv2D(16, 5×5) → ReLU → MaxPool(2×2)
- Dense(120) → ReLU
- Dense(84) → ReLU  
- Dense(10) → Softmax

### mnist_cnn.nwf
- **Architecture:** LeNet-5 variant
- **Dataset:** MNIST
- **Input:** 28×28 grayscale (legacy format)
- **Note:** Use lenet5_mnist.nwf for new projects

## Using Weight Files

In your `.nlang` network definition:

``````netlang
network MNIST {
    input(shape: [32, 32, 1])
    weights("models/lenet5_mnist.nwf")
    
    // ... layer definitions
}
``````

## Converting Your Own Models

See [tools/weight_converters/](../tools/weight_converters/) for conversion scripts.

### From PyTorch
``````bash
python tools/weight_converters/convert_pytorch.py \
    --model your_model.pth \
    --output models/your_model.nwf
``````

### From Keras
``````bash
python tools/weight_converters/convert_keras.py \
    --model your_model.h5 \
    --output models/your_model.nwf
``````

### From ONNX
``````bash
python tools/weight_converters/convert_onnx.py \
    --model your_model.onnx \
    --output models/your_model.nwf
``````

## Weight Format (.nwf)

NetLang uses a custom binary format optimized for:
- **Zero-copy mmap loading** (~1ms vs ~80ms for ONNX)
- **64-byte alignment** for AVX2 vectorization
- **Sequential layout** for cache efficiency

See [docs/weight-format-spec.md](../docs/weight-format-spec.md) for specification.

## File Size Guidelines

- Small networks (LeNet): < 1 MB - commit to repo
- Medium networks (VGG): 100-500 MB - external storage recommended
- Large networks (ResNet): < 1 GB - use Git LFS or external hosting

## .gitignore

By default, `.gitignore` excludes large `.nwf` files. Small models like `lenet5_mnist.nwf` are whitelisted for convenience.
"@
Set-Content -Path "models\README.md" -Value $modelsReadme

Write-Host"  Created: docs/README.md"
Write-Host "  Created: examples/README.md"
Write-Host "  Created: tools/README.md"
Write-Host "  Created: models/README.md"

Write-Host "`n" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Restructuring Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Yellow
Write-Host "1. Review the new structure: tree /F /A docs examples tools"
Write-Host "2. Test the build: make clean && make"
Write-Host "3. Update main README.md with new paths"
Write-Host "4. Update Makefile if paths changed"
Write-Host "5. Test example compilation: ./build/netlang examples/mnist_lenet5.nlang"
Write-Host "6. Verify tools work: python tools/weight_converters/convert_pytorch.py --help"
Write-Host "7. Clean up old directories if everything works"
Write-Host "8. Commit: git add . && git commit -m 'Restructure repository'"
Write-Host ""
Write-Host "Old directories you can remove after testing:" -ForegroundColor Yellow
Write-Host "  - architecture/"
Write-Host "  - explanation/"
Write-Host ""
