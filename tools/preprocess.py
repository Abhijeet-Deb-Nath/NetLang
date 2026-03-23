#!/usr/bin/env python3
"""
NetLang Image Preprocessor

Converts images to binary format (.bin) for NetLang inference.
Supports both single images and batch folder processing.

Usage:
    # Single image
    python tools/preprocess.py image.png output.bin
    
    # Entire folder
    python tools/preprocess.py assets/inputs/raw/ assets/inputs/preprocessed_custom/

Author: NetLang Compiler Project
Date: February 2026
"""

import sys
import os
from pathlib import Path
import numpy as np

try:
    from PIL import Image
except ImportError:
    print("Error: PIL (Pillow) not installed.")
    print("Install with: pip install Pillow")
    sys.exit(1)


def preprocess_mnist_image(image_path, target_size=(32, 32)):
    """
    Preprocess image for MNIST network.
    
    Args:
        image_path: Path to input image
        target_size: Target dimensions (height, width)
    
    Returns:
        numpy array of shape (H, W, 1) with float32 values in [0, 1]
    """
    # Load image
    img = Image.open(image_path)
    
    # Convert to grayscale
    if img.mode != 'L':
        img = img.convert('L')
    
    # Resize to target size
    img = img.resize(target_size, Image.BILINEAR)
    
    # Convert to numpy array and normalize to [0, 1]
    arr = np.array(img, dtype=np.float32)
    arr = arr / 255.0
    
    # Reshape to (H, W, 1) for consistency
    arr = arr.reshape(target_size[0], target_size[1], 1)
    
    return arr


def convert_single_image(input_path, output_path, verbose=True):
    """Convert a single image to .bin format"""
    try:
        # Preprocess
        arr = preprocess_mnist_image(input_path)
        
        # Save as binary
        arr.tofile(output_path)
        
        if verbose:
            size_kb = os.path.getsize(output_path) / 1024
            print(f"✓ {input_path.name} → {output_path.name} ({size_kb:.1f} KB)")
        
        return True
    
    except Exception as e:
        print(f"✗ Error processing {input_path}: {e}")
        return False


def convert_folder(input_dir, output_dir, extensions=('.png', '.jpg', '.jpeg', '.bmp')):
    """Convert all images in a folder to .bin format"""
    input_dir = Path(input_dir)
    output_dir = Path(output_dir)
    
    # Create output directory
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Find all image files
    image_files = []
    for ext in extensions:
        image_files.extend(input_dir.glob(f'**/*{ext}'))
        image_files.extend(input_dir.glob(f'**/*{ext.upper()}'))
    
    if not image_files:
        print(f"No image files found in {input_dir}")
        return 0
    
    print(f"Found {len(image_files)} images in {input_dir}")
    print(f"Converting to {output_dir}...")
    print("-" * 60)
    
    # Convert each image
    success_count = 0
    for img_file in sorted(image_files):
        output_file = output_dir / (img_file.stem + '.bin')
        if convert_single_image(img_file, output_file, verbose=True):
            success_count += 1
    
    print("-" * 60)
    print(f"✓ Successfully converted {success_count}/{len(image_files)} images")
    
    return success_count


def print_usage():
    """Print usage instructions"""
    print(__doc__)
    print("\nExamples:")
    print("  # Convert single image")
    print("  python tools/preprocess.py digit_7.png output.bin")
    print()
    print("  # Convert entire folder")
    print("  python tools/preprocess.py assets/inputs/raw/ assets/inputs/preprocessed_custom/")
    print()
    print("Supported formats: PNG, JPG, JPEG, BMP")
    print("Output: 32×32 grayscale float32 binary (.bin)")


def main():
    if len(sys.argv) < 3:
        print_usage()
        sys.exit(1)
    
    input_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])
    
    if not input_path.exists():
        print(f"Error: Input path '{input_path}' does not exist")
        sys.exit(1)
    
    # Determine mode: single file or folder
    if input_path.is_file():
        # Single image mode
        print(f"Converting single image: {input_path}")
        success = convert_single_image(input_path, output_path)
        sys.exit(0 if success else 1)
    
    elif input_path.is_dir():
        # Folder mode
        count = convert_folder(input_path, output_path)
        sys.exit(0 if count > 0 else 1)
    
    else:
        print(f"Error: '{input_path}' is neither a file nor a directory")
        sys.exit(1)


if __name__ == '__main__':
    main()
