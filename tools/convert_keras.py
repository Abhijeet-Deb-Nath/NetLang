#!/usr/bin/env python3
"""
NetLang Weight Converter - TensorFlow/Keras to .nwf

Converts TensorFlow/Keras model weights (.h5, SavedModel) to NetLang Weight Format (.nwf)
with optimal memory layout and 64-byte alignment for AVX2 performance.

Author: Abhijeet Deb Nath
Date: February 2026

Usage:
    python convert_keras.py --model vgg16.h5 --output vgg16.nwf
    python convert_keras.py --model saved_model/ --output model.nwf --verbose
"""

import argparse
import struct
import sys
import os
from typing import List, Tuple
import numpy as np

try:
    import tensorflow as tf
    from tensorflow import keras
except ImportError:
    print("Error: TensorFlow not installed. Run: pip install tensorflow")
    sys.exit(1)


# Layer type constants
LAYER_TYPE_CONV2D = 0
LAYER_TYPE_DENSE = 1
LAYER_TYPE_BATCHNORM = 2

# Data type constants
DTYPE_FLOAT32 = 0


class LayerInfo:
    """Information about a single layer"""
    def __init__(self, name: str, layer_type: int, weights: np.ndarray, bias: np.ndarray = None):
        self.name = name
        self.layer_type = layer_type
        self.weights = weights
        self.bias = bias if bias is not None else np.zeros(weights.shape[-1] if layer_type == LAYER_TYPE_CONV2D else weights.shape[0], dtype=np.float32)
        
    def weight_shape(self) -> Tuple[int, int, int, int]:
        """Return weight shape as 4D tuple"""
        shape = self.weights.shape
        if len(shape) == 4:
            return shape
        elif len(shape) == 2:
            return (shape[0], shape[1], 1, 1)
        elif len(shape) == 1:
            return (shape[0], 1, 1, 1)
        else:
            raise ValueError(f"Unexpected weight shape: {shape}")


def align_to_64(offset: int) -> int:
    """Align offset to 64-byte boundary"""
    return (offset + 63) & ~63


def extract_layers_from_keras_model(model: keras.Model) -> List[LayerInfo]:
    """
    Extract Conv2D and Dense layers from Keras model.
    Handles both Sequential and Functional API models.
    """
    layers = []
    
    for layer in model.layers:
        if isinstance(layer, keras.layers.Conv2D):
            weights = layer.get_weights()
            if len(weights) >= 2:
                # Keras Conv2D weights: [kernel_h, kernel_w, in_channels, out_channels]
                w = weights[0].astype(np.float32)
                b = weights[1].astype(np.float32)
            else:
                w = weights[0].astype(np.float32)
                b = np.zeros(w.shape[-1], dtype=np.float32)
            
            layers.append(LayerInfo(layer.name, LAYER_TYPE_CONV2D, w, b))
            
        elif isinstance(layer, keras.layers.Dense):
            weights = layer.get_weights()
            if len(weights) >= 2:
                # Keras Dense weights: [in_features, out_features]
                # Need to transpose to [out_features, in_features] for our format
                w = weights[0].astype(np.float32).T  # Transpose!
                b = weights[1].astype(np.float32)
            else:
                w = weights[0].astype(np.float32).T
                b = np.zeros(w.shape[0], dtype=np.float32)
            
            layers.append(LayerInfo(layer.name, LAYER_TYPE_DENSE, w, b))
            
        elif isinstance(layer, keras.layers.BatchNormalization):
            weights = layer.get_weights()
            if len(weights) >= 2:
                # BatchNorm: [gamma, beta, moving_mean, moving_variance]
                gamma = weights[0].astype(np.float32)
                beta = weights[1].astype(np.float32)
                layers.append(LayerInfo(layer.name, LAYER_TYPE_BATCHNORM, gamma, beta))
    
    return layers


def load_keras_model(model_path: str) -> keras.Model:
    """Load Keras model from file"""
    if not os.path.exists(model_path):
        raise FileNotFoundError(f"Model file not found: {model_path}")
    
    print(f"Loading Keras model: {model_path}")
    
    try:
        if model_path.endswith('.h5'):
            model = keras.models.load_model(model_path)
        else:
            # Try loading as SavedModel directory
            model = keras.models.load_model(model_path)
        
        return model
    except Exception as e:
        print(f"Error loading model: {e}")
        sys.exit(1)


def write_nwf_file(layers: List[LayerInfo], output_path: str, verbose: bool = False):
    """Write layers to .nwf format"""
    
    print(f"\nConverting {len(layers)} layers to .nwf format...")
    
    with open(output_path, 'wb') as f:
        # ========== WRITE HEADER ==========
        magic = b'NWGT'
        version = 1
        layer_count = len(layers)
        alignment = 64
        dtype = DTYPE_FLOAT32
        metadata_offset = 256
        
        # Header structure (256 bytes)
        f.write(magic)                                    # 4 bytes: magic
        f.write(struct.pack('<I', version))              # 4 bytes: version
        f.write(struct.pack('<I', layer_count))          # 4 bytes: layer count
        f.write(struct.pack('<Q', 0))                    # 8 bytes: total_size (placeholder)
        f.write(struct.pack('<I', alignment))            # 4 bytes: alignment
        f.write(struct.pack('<I', dtype))                # 4 bytes: dtype
        f.write(struct.pack('<Q', metadata_offset))      # 8 bytes: metadata offset
        
        # Calculate data offset
        data_offset = metadata_offset + (layer_count * 64)
        data_offset = align_to_64(data_offset)
        f.write(struct.pack('<Q', data_offset))          # 8 bytes: data offset
        f.write(bytes(212))                              # 212 bytes: reserved
        
        # ========== PREPARE METADATA ==========
        metadata = []
        current_offset = data_offset
        
        for i, layer in enumerate(layers):
            weights = layer.weights.astype(np.float32)
            
            # Calculate sizes
            weight_size = weights.nbytes
            bias_size = layer.bias.nbytes
            
            # Align weight offset
            weight_offset = align_to_64(current_offset)
            current_offset = weight_offset + weight_size
            
            # Align bias offset
            bias_offset = align_to_64(current_offset)
            current_offset = bias_offset + bias_size
            
            # Store metadata
            weight_shape = layer.weight_shape()
            metadata.append({
                'layer_type': layer.layer_type,
                'layer_id': i,
                'weight_offset': weight_offset,
                'weight_size': weight_size,
                'weight_shape': weight_shape,
                'bias_offset': bias_offset,
                'bias_size': bias_size,
                'bias_shape': layer.bias.shape[0],
                'weights': weights,
                'bias': layer.bias
            })
            
            if verbose:
                print(f"  [{i}] {layer.name}")
                print(f"      Type: {['Conv2D', 'Dense', 'BatchNorm'][layer.layer_type]}")
                print(f"      Weights: {weight_shape} ({weight_size:,} bytes)")
                print(f"      Bias: [{layer.bias.shape[0]}] ({bias_size:,} bytes)")
        
        total_size = current_offset
        
        # ========== WRITE METADATA TABLE ==========
        for meta in metadata:
            f.write(struct.pack('<I', meta['layer_type']))           # 4 bytes
            f.write(struct.pack('<I', meta['layer_id']))             # 4 bytes
            f.write(struct.pack('<Q', meta['weight_offset']))        # 8 bytes
            f.write(struct.pack('<Q', meta['weight_size']))          # 8 bytes
            f.write(struct.pack('<IIII', *meta['weight_shape']))     # 16 bytes
            f.write(struct.pack('<Q', meta['bias_offset']))          # 8 bytes
            f.write(struct.pack('<Q', meta['bias_size']))            # 8 bytes
            f.write(struct.pack('<I', meta['bias_shape']))           # 4 bytes
            f.write(bytes(16))                                        # 16 bytes: reserved
        
        # Pad to data_offset
        current_pos = f.tell()
        padding = data_offset - current_pos
        if padding > 0:
            f.write(bytes(padding))
        
        # ========== WRITE WEIGHT DATA ==========
        for meta in metadata:
            # Write weights (aligned)
            current_pos = f.tell()
            padding = meta['weight_offset'] - current_pos
            if padding > 0:
                f.write(bytes(padding))
            
            f.write(meta['weights'].tobytes())
            
            # Write bias (aligned)
            current_pos = f.tell()
            padding = meta['bias_offset'] - current_pos
            if padding > 0:
                f.write(bytes(padding))
            
            f.write(meta['bias'].astype(np.float32).tobytes())
        
        # ========== UPDATE HEADER WITH TOTAL SIZE ==========
        f.seek(8)
        f.write(struct.pack('<Q', total_size))
    
    # Print summary
    print(f"\n✓ Conversion complete!")
    print(f"  Output file: {output_path}")
    print(f"  Total size: {total_size:,} bytes ({total_size / (1024**2):.2f} MB)")
    print(f"  Layers: {len(layers)}")
    print(f"  Alignment: {alignment} bytes")


def main():
    parser = argparse.ArgumentParser(
        description="Convert Keras/TensorFlow model weights to NetLang .nwf format",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python convert_keras.py --model vgg16.h5 --output vgg16.nwf
  python convert_keras.py --model saved_model/ --output model.nwf --verbose

Notes:
  - Extracts Conv2D, Dense, and BatchNormalization layers
  - Transposes Dense layer weights for cache efficiency
  - Ensures 64-byte alignment for AVX2 performance
  - Supports both .h5 and SavedModel formats
        """
    )
    
    parser.add_argument('--model', required=True, help='Input Keras model (.h5 or SavedModel directory)')
    parser.add_argument('--output', required=True, help='Output .nwf file')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    # Load model
    model = load_keras_model(args.model)
    
    # Extract layers
    layers = extract_layers_from_keras_model(model)
    
    if len(layers) == 0:
        print("Error: No Conv2D or Dense layers found in model")
        sys.exit(1)
    
    print(f"Found {len(layers)} layers:")
    for i, layer in enumerate(layers):
        layer_type_name = ['Conv2D', 'Dense', 'BatchNorm'][layer.layer_type]
        print(f"  {i}: {layer.name} ({layer_type_name}) {layer.weight_shape()}")
    
    # Convert to .nwf
    write_nwf_file(layers, args.output, args.verbose)


if __name__ == '__main__':
    main()
