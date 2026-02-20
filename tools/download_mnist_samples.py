#!/usr/bin/env python3
"""
Download Sample MNIST Test Images

This script downloads a few sample MNIST images for testing your NetLang system.
Images are saved to test_data/images/

Usage:
    python tools/download_mnist_samples.py

Requirements:
    pip install requests pillow numpy
"""

import sys
import os
from pathlib import Path

try:
    import numpy as np
    from PIL import Image
    import requests
except ImportError:
    print("Error: Missing dependencies")
    print("Install with: pip install requests pillow numpy")
    sys.exit(1)


def download_mnist_samples(output_dir='test_data/images', num_samples=20):
    """
    Download sample MNIST test images.
    
    Note: This downloads from a simple source. For full MNIST dataset,
    use `pip install mnist` or download from http://yann.lecun.com/exdb/mnist/
    """
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"Downloading {num_samples} MNIST sample images...")
    print(f"Saving to: {output_dir}")
    print("-" * 60)
    
    try:
        # Try to import mnist package
        import mnist
        
        # Load MNIST test set
        test_images = mnist.test_images()
        test_labels = mnist.test_labels()
        
        # Save first num_samples images
        for i in range(min(num_samples, len(test_images))):
            img_array = test_images[i]
            label = test_labels[i]
            
            # Convert to PIL Image
            img = Image.fromarray(img_array, mode='L')
            
            # Save as PNG
            output_path = output_dir / f"mnist_{i:03d}_label_{label}.png"
            img.save(output_path)
            
            print(f"✓ Saved: {output_path.name} (label: {label})")
        
        print("-" * 60)
        print(f"✓ Successfully downloaded {num_samples} images!")
        print(f"\nNext steps:")
        print(f"  1. Preprocess: python tools/preprocess.py {output_dir} test_data/preprocessed/")
        print(f"  2. Compile test harness: See instructions below")
        print(f"  3. Run inference: bin/test_network.exe test_data/preprocessed/mnist_000_label_7.bin")
        
        return True
        
    except ImportError:
        print("Error: 'mnist' package not installed")
        print("\nInstall with:")
        print("  pip install mnist")
        print("\nAlternatively, manually download MNIST images or use your own 28x28 grayscale images")
        return False


def create_sample_digit_manually(output_dir='test_data/images'):
    """
    Create a simple synthetic digit image for testing (fallback)
    """
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Create a simple "7" digit
    img = np.zeros((28, 28), dtype=np.uint8)
    
    # Draw a 7
    img[5:8, 5:23] = 255    # Top horizontal line
    img[5:20, 20:23] = 255  # Diagonal line
    
    # Save
    output_path = output_dir / "sample_digit_7.png"
    Image.fromarray(img, mode='L').save(output_path)
    
    print(f"✓ Created sample image: {output_path}")
    print("\nThis is a synthetic test image. For real MNIST data:")
    print("  pip install mnist")
    print("  Then run this script again")


if __name__ == '__main__':
    success = download_mnist_samples()
    
    if not success:
        print("\nCreating synthetic test image instead...")
        create_sample_digit_manually()
