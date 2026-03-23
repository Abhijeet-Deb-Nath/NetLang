#!/usr/bin/env python3
"""
Regenerate MNIST test data at 28x28 for ONNX-trained model

Usage:
    python tools/regenerate_mnist_28x28.py
"""

import sys
from pathlib import Path

try:
    import numpy as np
    from torchvision import datasets
except ImportError as e:
    print(f"Error: Missing dependencies - {e}")
    print("\nInstall with:")
    print("  pip install torch torchvision numpy")
    sys.exit(1)


def main():
    output_dir = Path('assets/inputs/preprocessed_28x28')
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print("=" * 70)
    print("Generating 28x28 MNIST test data")
    print("=" * 70)
    
    # Load MNIST test set
    mnist_test = datasets.MNIST(root='./assets/datasets', train=False, download=True)
    
    print(f"Converting to .bin format (28×28 grayscale, float32, [0,1])...")
    print(f"Output: {output_dir}/")
    
    # Convert images
    for i, (img, label) in enumerate(mnist_test):
        # Keep at 28×28 (original size)
        arr = np.array(img, dtype=np.float32) / 255.0
        arr = arr.reshape(28, 28, 1)  # Add channel dimension
        
        # Save as binary file
        output_path = output_dir / f"mnist_{i:04d}_label_{label}.bin"
        arr.tofile(output_path)
        
        if (i + 1) % 1000 == 0:
            print(f"  Processed {i + 1}/10000 images...")
    
    print(f"✓ Created 10,000 28x28 .bin files")
    print(f"  File size: {28*28*4} bytes each")


if __name__ == '__main__':
    main()
