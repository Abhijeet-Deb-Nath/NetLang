# Test Data Directory

This directory contains preprocessed test data for NetLang inference.

## Directory Structure

```
test_data/
├── preprocessed/    # Binary files ready for inference (.bin)
└── README.md        # This file
```

## Quick Start Workflow

### 1. Download MNIST Test Data (Direct to .bin format!)

```bash
python tools/download_mnist_for_netlang.py
```

This downloads **all 10,000 MNIST test images** and converts directly to `.bin` format in one step!

**Requirements:**
```bash
pip install torch torchvision numpy
```

### 2. Run Inference

Compile and run the test harness:

```bash
# Compile test harness
gcc -O3 -march=haswell -mavx2 -mfma ^
    generated/mnist.c ^
    src/codegen/runtime.c ^
    src/codegen/kernels.c ^
    tools/test_network.c ^
    -o bin/test_network.exe

# Run inference
bin/test_network.exe test_data/preprocessed/digit_7.bin
```

## Expected Output

```
NetLang Inference
=================
Input file: test_data/preprocessed/digit_7.bin
Loading input... OK (1024 floats)
Initializing network... OK
Running inference... OK (4.23 ms)

=================
RESULT: 7
=================
Confidence: 98.45%
Inference time: 4.23 ms

Probability distribution:
  Class 0:  0.02% |
  Class 1:  0.15% |
  Class 2:  0.08% |
  Class 3:  0.12% |
  Class 4:  0.05% |
  Class 5:  0.03% |
  Class 6:  0.10% |
  Class 7: 98.45% |████████████████████████████████████████████████
  Class 8:  0.80% |
  Class 9:  0.20% |

Done!
```

## File Formats

### Preprocessed Binary (.bin)
- **Format:** Raw float32 binary
- **Shape:** 32 × 32 × 1 = 1024 floats
- **Size:** 4096 bytes (1024 floats × 4 bytes)
- **Range:** [0.0, 1.0] (normalized pixel values)
- **Layout:** Row-major (HWC format)

## Using Custom Images (Not MNIST)

If you want to test with your own images instead of MNIST:

```bash
# Install dependencies
pip install Pillow numpy

# Single image
python tools/preprocess.py my_digit.png test_data/preprocessed/my_digit.bin

# Entire folder
python tools/preprocess.py my_images/ test_data/preprocessed/
```

**Supported formats:** PNG, JPG, JPEG, BMP  
**Input requirements:** Any size (resized to 32×32), any color (converted to grayscale)

## Batch Processing

For benchmarking with all images:

```bash
# Run inference on all 10,000 test images (PowerShell)
Get-ChildItem test_data\preprocessed\*.bin | ForEach-Object {
    bin\test_network.exe $_.FullName
}
```

## Disk Space Management

```
data/                    55 MB (PyTorch cache - can be deleted)
test_data/preprocessed/  40 MB (10,000 .bin files - needed for inference)
```

After conversion, you can delete `data/` to save 55 MB. It will re-download
if you run the script again.

## Git Ignore

These folders are in `.gitignore`:
- `test_data/images/*.png` - Large binary files
- `test_data/preprocessed/*.bin` - Generated files

Only the folder structure is tracked.

## Notes

- Preprocessing is done **once** per image
- `.bin` files can be reused for multiple inference runs
- This matches industry practice (TVM, ONNX Runtime, etc.)
- For production, you might preprocess on-the-fly or use C preprocessing
