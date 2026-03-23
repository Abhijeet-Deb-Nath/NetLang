#!/usr/bin/env python3
"""
Download MNIST Test Data for NetLang

Downloads all 10,000 MNIST test images and converts them directly to .bin format.
No manual preprocessing needed!

Usage:
    python tools/download_mnist_for_netlang.py [--size 28|32]
    
    Default: 28x28 (native MNIST size, matches ONNX models)

Output:
    assets/inputs/preprocessed_28x28/*.bin or assets/inputs/preprocessed_32x32/*.bin
    files ready for inference

Requirements:
    pip install torch torchvision numpy
"""

import sys
import argparse
from pathlib import Path

try:
    import numpy as np
    from torchvision import datasets
    from PIL import Image
except ImportError as e:
    print(f"Error: Missing dependencies - {e}")
    print("\nInstall with:")
    print("  pip install torch torchvision numpy")
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description='Download MNIST for NetLang')
    parser.add_argument('--size', type=int, default=28, choices=[28, 32],
                        help='Image size: 28 (native MNIST) or 32 (LeNet5)')
    args = parser.parse_args()
    
    img_size = args.size
    output_dir = Path(f'assets/inputs/preprocessed_{img_size}x{img_size}')
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print("=" * 70)
    print("NetLang MNIST Dataset Downloader")
    print("=" * 70)
    print("Downloading all 10,000 MNIST test images...")
    print("(First run downloads ~10MB from http://yann.lecun.com/exdb/mnist/)")
    print()
    
    # Download MNIST test set
    mnist_test = datasets.MNIST(root='./assets/datasets', train=False, download=True)
    
    print(f"Converting to .bin format ({img_size}×{img_size} grayscale, float32, [0,1])...")
    print(f"Output: {output_dir}/")
    print("-" * 70)
    
    # Convert all images
    for i, (img, label) in enumerate(mnist_test):
        # Resize only if not using native 28×28
        if img_size != 28:
            img = img.resize((img_size, img_size), Image.BILINEAR)
        
        # Convert to normalized float32 array
        arr = np.array(img, dtype=np.float32) / 255.0
        arr = arr.reshape(img_size, img_size, 1)  # Add channel dimension
        
        # Save as binary file
        output_path = output_dir / f"mnist_{i:04d}_label_{label}.bin"
        arr.tofile(output_path)
        
        # Progress update every 1000 images
        if (i + 1) % 1000 == 0:
            print(f"  Processed {i + 1}/10000 images...")
    
    file_size_mb = (10000 * img_size * img_size * 4) / (1024 * 1024)
    print("-" * 70)
    print(f"✓ Successfully created 10,000 .bin files (~{file_size_mb:.0f} MB)")
    print(f"  Format: {img_size}×{img_size}×1 float32")
    print()
    if img_size == 28:
        print("Quick test:")
        print(f"  bin\\lenet5_smoke.exe {output_dir}\\mnist_0000_label_7.bin")
        print()
    else:
        print("Use these files with a network that expects 32x32 inputs.")
        print()
    print("Dataset cache lives under assets/datasets/.")
    print("=" * 70)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)
