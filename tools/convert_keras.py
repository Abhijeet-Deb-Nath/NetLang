#!/usr/bin/env python3
"""
TensorFlow/Keras to NetLang Weight Format (.nwf) Converter

Converts Keras .h5 or SavedModel files to optimized .nwf format for NetLang compiler.
Supports Conv2D, Dense, BatchNormalization layers.

Usage:
    python convert_keras.py --model path/to/model.h5 --output weights.nwf
    
Example:
    python convert_keras.py --model vgg16.h5 --output models/vgg16.nwf
"""

import numpy as np
import struct
import argparse
import sys
from pathlib import Path
from typing import List, Dict, Tuple

try:
    import tensorflow as tf
    from tensorflow import keras
except ImportError:
    print("Error: TensorFlow not installed. Install with: pip install tensorflow")
    sys.exit(1)

# Layer type codes
LAYER_CONV2D = 0
LAYER_DENSE = 1
LAYER_BATCHNORM = 2
LAYER_LAYERNORM = 3

DTYPE_FLOAT32 = 0
ALIGNMENT = 64


class LayerMetadata:
    """Represents metadata for a single layer"""
    def __init__(self, layer_type: int, layer_id: int):
        self.layer_type = layer_type
        self.layer_id = layer_id
        self.weight_offset = 0
        self.weight_size = 0
        self.weight_shape = [0, 0, 0, 0]
        self.bias_offset = 0
        self.bias_size = 0
        self.bias_shape = [0]
        
    def to_bytes(self) -> bytes:
        """Serialize metadata to 64-byte structure"""
        data = struct.pack(
            '<IIQQIIIIQQI',
            self.layer_type,
            self.layer_id,
            self.weight_offset,
            self.weight_size,
            self.weight_shape[0],
            self.weight_shape[1],
            self.weight_shape[2],
            self.weight_shape[3],
            self.bias_offset,
            self.bias_size,
            self.bias_shape[0]
        )
        padding = b'\x00' * (64 - len(data))
        return data + padding


def align_to(value: int, alignment: int) -> int:
    """Round up to next alignment boundary"""
    return (value + alignment - 1) & ~(alignment - 1)


def write_aligned_array(f, array: np.ndarray) -> Tuple[int, int]:
    """Write numpy array with 64-byte alignment, return (offset, size)"""
    current_pos = f.tell()
    aligned_pos = align_to(current_pos, ALIGNMENT)
    if aligned_pos > current_pos:
        f.write(b'\x00' * (aligned_pos - current_pos))
    
    offset = f.tell()
    data = array.astype(np.float32).tobytes()
    f.write(data)
    size = len(data)
    
    return offset, size


def extract_keras_layers(model) -> List[Dict]:
    """
    Extract layer weights from Keras model
    
    Args:
        model: Keras model instance
        
    Returns:
        List of layer dictionaries with weights, biases, types
    """
    layers = []
    layer_id = 0
    
    for layer in model.layers:
        weights = layer.get_weights()
        if not weights:
            continue  # Skip layers without weights (e.g., activations, pooling)
        
        layer_name = layer.name
        layer_class = layer.__class__.__name__
        
        if layer_class == 'Conv2D':
            # Keras Conv2D weights: [kernel_h, kernel_w, in_channels, out_channels]
            # Already in correct format for NetLang!
            weight = weights[0]
            bias = weights[1] if len(weights) > 1 else None
            
            layers.append({
                'name': layer_name,
                'type': LAYER_CONV2D,
                'weight': weight,
                'weight_shape': list(weight.shape),
                'bias': bias,
                'bias_shape': [len(bias)] if bias is not None else [0]
            })
            layer_id += 1
            
        elif layer_class == 'Dense':
            # Keras Dense weights: [in_features, out_features]
            # Already in correct format for NetLang!
            weight = weights[0]
            bias = weights[1] if len(weights) > 1 else None
            
            layers.append({
                'name': layer_name,
                'type': LAYER_DENSE,
                'weight': weight,
                'weight_shape': list(weight.shape) + [0, 0],
                'bias': bias,
                'bias_shape': [len(bias)] if bias is not None else [0]
            })
            layer_id += 1
            
        elif layer_class == 'BatchNormalization':
            # Keras BN: [gamma, beta, moving_mean, moving_var]
            # We only need gamma (weight) and beta (bias) for inference
            gamma = weights[0]  # Scale
            beta = weights[1]   # Shift
            
            layers.append({
                'name': layer_name,
                'type': LAYER_BATCHNORM,
                'weight': gamma,
                'weight_shape': [len(gamma), 0, 0, 0],
                'bias': beta,
                'bias_shape': [len(beta)]
            })
            layer_id += 1
            
        elif layer_class == 'LayerNormalization':
            gamma = weights[0]
            beta = weights[1] if len(weights) > 1 else None
            
            layers.append({
                'name': layer_name,
                'type': LAYER_LAYERNORM,
                'weight': gamma,
                'weight_shape': [len(gamma), 0, 0, 0],
                'bias': beta,
                'bias_shape': [len(beta)] if beta is not None else [0]
            })
            layer_id += 1
        else:
            print(f"Warning: Skipping unsupported layer type: {layer_class} ({layer_name})")
    
    return layers


def convert_keras_to_nwf(model_path: str, output_path: str):
    """
    Convert Keras model to .nwf format
    
    Args:
        model_path: Path to .h5 file or SavedModel directory
        output_path: Output .nwf file path
    """
    print(f"Loading Keras model: {model_path}")
    
    try:
        model = keras.models.load_model(model_path)
    except Exception as e:
        print(f"Error loading model: {e}")
        sys.exit(1)
    
    print(f"Model architecture: {model.name}")
    print(f"Total layers: {len(model.layers)}")
    
    print(f"Extracting layers...")
    layers = extract_keras_layers(model)
    
    if not layers:
        print("Error: No valid layers found in model")
        sys.exit(1)
    
    print(f"Found {len(layers)} layers with weights:")
    for layer in layers:
        layer_type_name = ['Conv2D', 'Dense', 'BatchNorm', 'LayerNorm'][layer['type']]
        print(f"  {layer['name']}: {layer_type_name} {layer['weight_shape']}")
    
    # Create output directory
    Path(output_path).parent.mkdir(parents=True, exist_ok=True)
    
    print(f"Writing .nwf file: {output_path}")
    
    with open(output_path, 'wb') as f:
        # Reserve header space
        header_pos = f.tell()
        f.write(b'\x00' * 256)
        
        # Reserve metadata table space
        metadata_offset = f.tell()
        metadata_list = []
        for idx in range(len(layers)):
            f.write(b'\x00' * 64)
        
        # Align data section
        data_offset = align_to(f.tell(), ALIGNMENT)
        f.seek(data_offset)
        
        # Write weights and collect metadata
        for idx, layer in enumerate(layers):
            meta = LayerMetadata(layer['type'], idx)
            
            # Write weights
            meta.weight_offset, meta.weight_size = write_aligned_array(f, layer['weight'])
            meta.weight_shape = layer['weight_shape']
            
            # Write bias if present
            if layer['bias'] is not None:
                meta.bias_offset, meta.bias_size = write_aligned_array(f, layer['bias'])
                meta.bias_shape = layer['bias_shape']
            else:
                meta.bias_offset = 0
                meta.bias_size = 0
                meta.bias_shape = [0]
            
            metadata_list.append(meta)
        
        total_size = f.tell()
        
        # Write metadata table
        f.seek(metadata_offset)
        for meta in metadata_list:
            f.write(meta.to_bytes())
        
        # Write header
        f.seek(header_pos)
        header = struct.pack(
            '<4sIIQIIQQ212s',
            b'NWGT',
            1,
            len(layers),
            total_size,
            ALIGNMENT,
            DTYPE_FLOAT32,
            metadata_offset,
            data_offset,
            b'\x00' * 212
        )
        f.write(header)
    
    print(f"✓ Conversion complete!")
    print(f"  Output file: {output_path}")
    print(f"  Total size: {total_size:,} bytes ({total_size / 1024 / 1024:.2f} MB)")
    print(f"  Layers: {len(layers)}")


def main():
    parser = argparse.ArgumentParser(
        description='Convert Keras models to NetLang Weight Format (.nwf)',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Convert Keras .h5 model
  python convert_keras.py --model vgg16.h5 --output models/vgg16.nwf
  
  # Convert SavedModel
  python convert_keras.py --model saved_model/ --output models/model.nwf
        """
    )
    
    parser.add_argument('--model', '-m', required=True,
                        help='Path to Keras .h5 file or SavedModel directory')
    parser.add_argument('--output', '-o', required=True,
                        help='Output .nwf file path')
    
    args = parser.parse_args()
    
    model_path = Path(args.model)
    if not model_path.exists():
        print(f"Error: Model not found: {args.model}")
        sys.exit(1)
    
    convert_keras_to_nwf(args.model, args.output)


if __name__ == '__main__':
    main()
