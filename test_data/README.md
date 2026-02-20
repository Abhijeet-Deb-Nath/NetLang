# Test Data Directory

This directory contains test images and preprocessed data for NetLang inference.

## Directory Structure

```
test_data/
├── images/          # Original test images (PNG, JPG, etc.)
├── preprocessed/    # Converted binary files (.bin)
└── README.md        # This file
```

## Workflow

### 1. Add Test Images

Put your test images in `test_data/images/`:

```bash
# Download MNIST samples
python tools/download_mnist_samples.py

# Or copy your own images
cp my_digit.png test_data/images/
```

### 2. Preprocess Images

Convert images to binary format for NetLang:

```bash
# Single image
python tools/preprocess.py test_data/images/digit_7.png test_data/preprocessed/digit_7.bin

# Entire folder
python tools/preprocess.py test_data/images/ test_data/preprocessed/
```

### 3. Run Inference

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

### Input Images
- **Supported:** PNG, JPG, JPEG, BMP
- **Any size:** Will be resized to 32×32
- **Any color:** Will be converted to grayscale

### Preprocessed Binary (.bin)
- **Format:** Raw float32 binary
- **Shape:** 32 × 32 × 1 = 1024 floats
- **Size:** 4096 bytes (1024 floats × 4 bytes)
- **Range:** [0.0, 1.0] (normalized pixel values)
- **Layout:** Row-major (HWC format)

## Batch Processing

For benchmarking, preprocess many images at once:

```bash
# Preprocess 1000 images
python tools/preprocess.py test_data/images/ test_data/preprocessed/

# Benchmark inference speed
time for f in test_data/preprocessed/*.bin; do
    bin/test_network.exe $f
done
```

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
