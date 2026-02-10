#!/usr/bin/env python3
"""
PyTorch to NetLang Weight Format (.nwf) Converter

Converts PyTorch .pth/.pt model files to optimized .nwf format for NetLang compiler.
Supports Conv2D, Linear (Dense), BatchNorm layers.

Usage:
    python convert_pytorch.py --model path/to/model.pth --output weights.nwf [--arch mnist]
    
Example:
    python convert_pytorch.py --model mnist_cnn.pth --output models/mnist.nwf --arch mnist
"""

import torch
import numpy as np
import struct
import argparse
import sys
from pathlib import Path
from typing import List, Dict, Tuple

# Layer type codes (matching WEIGHT_FORMAT.md)
LAYER_CONV2D = 0
LAYER_DENSE = 1
LAYER_BATCHNORM = 2
LAYER_LAYERNORM = 3

# Data type codes
DTYPE_FLOAT32 = 0
DTYPE_FLOAT16 = 1

ALIGNMENT = 64  # AVX2 cache line alignment


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
            '<IIQQIIIIQQI',  # Little-endian format
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
        # Pad to 64 bytes
        padding = b'\x00' * (64 - len(data))
        return data + padding


def align_to(value: int, alignment: int) -> int:
    """Round up to next alignment boundary"""
    return (value + alignment - 1) & ~(alignment - 1)


def write_aligned_array(f, array: np.ndarray) -> Tuple[int, int]:
    """Write numpy array with 64-byte alignment, return (offset, size)"""
    # Ensure current position is aligned
    current_pos = f.tell()
    aligned_pos = align_to(current_pos, ALIGNMENT)
    if aligned_pos > current_pos:
        f.write(b'\x00' * (aligned_pos - current_pos))
    
    offset = f.tell()
    
    # Write array data
    data = array.astype(np.float32).tobytes()
    f.write(data)
    size = len(data)
    
    return offset, size


def extract_pytorch_layers(model_dict: Dict, architecture: str = None) -> List[Dict]:
    """
    Extract layer weights from PyTorch state_dict
    
    Args:
        model_dict: PyTorch state_dict
        architecture: Optional architecture hint (mnist, vgg, resnet, etc.)
        
    Returns:
        List of layer dictionaries with weights, biases, types
    """
    layers = []
    
    # Group weights and biases by layer
    layer_map = {}
    for name, param in model_dict.items():
        # Parse layer name (e.g., "conv1.weight", "fc.bias")
        parts = name.split('.')
        layer_name = '.'.join(parts[:-1])
        param_type = parts[-1]  # 'weight' or 'bias'
        
        if layer_name not in layer_map:
            layer_map[layer_name] = {}
        
        layer_map[layer_name][param_type] = param.cpu().numpy()
    
    # Convert to layer list
    for idx, (layer_name, params) in enumerate(sorted(layer_map.items())):
        if 'weight' not in params:
            continue  # Skip layers without weights (e.g., activations, pooling)
            
        weight = params['weight']
        bias = params.get('bias', None)
        
        # Determine layer type from weight shape
        if len(weight.shape) == 4:
            # Conv2D: [out_channels, in_channels, kernel_h, kernel_w] (PyTorch format)
            # Need to transpose to NetLang format: [kernel_h, kernel_w, in_channels, out_channels]
            layer_type = LAYER_CONV2D
            weight_transposed = np.transpose(weight, (2, 3, 1, 0))  # [Kh, Kw, Cin, Cout]
            weight_shape = list(weight_transposed.shape)
            
        elif len(weight.shape) == 2:
            # Dense/Linear: [out_features, in_features] (PyTorch format)
            # Need to transpose to NetLang format: [in_features, out_features]
            layer_type = LAYER_DENSE
            weight_transposed = np.transpose(weight, (1, 0))  # [In, Out]
            weight_shape = list(weight_transposed.shape) + [0, 0]
            
        elif len(weight.shape) == 1:
            # BatchNorm/LayerNorm: [channels]
            if 'bn' in layer_name.lower() or 'batch' in layer_name.lower():
                layer_type = LAYER_BATCHNORM
            else:
                layer_type = LAYER_LAYERNORM
            weight_transposed = weight
            weight_shape = [len(weight), 0, 0, 0]
        else:
            print(f"Warning: Skipping unsupported layer {layer_name} with shape {weight.shape}")
            continue
        
        layers.append({
            'name': layer_name,
            'type': layer_type,
            'weight': weight_transposed,
            'weight_shape': weight_shape,
            'bias': bias,
            'bias_shape': [len(bias)] if bias is not None else [0]
        })
    
    return layers


def convert_pytorch_to_nwf(model_path: str, output_path: str, architecture: str = None):
    """
    Convert PyTorch model to .nwf format
    
    Args:
        model_path: Path to .pth or .pt file
        output_path: Output .nwf file path
        architecture: Optional architecture hint
    """
    print(f"Loading PyTorch model: {model_path}")
    
    # Load model (handles both state_dict and full model saves)
    try:
        checkpoint = torch.load(model_path, map_location='cpu')
        if isinstance(checkpoint, dict) and 'state_dict' in checkpoint:
            model_dict = checkpoint['state_dict']
        elif isinstance(checkpoint, dict):
            model_dict = checkpoint
        else:
            # Assume it's a full model
            model_dict = checkpoint.state_dict()
    except Exception as e:
        print(f"Error loading model: {e}")
        sys.exit(1)
    
    print(f"Extracting layers...")
    layers = extract_pytorch_layers(model_dict, architecture)
    
    if not layers:
        print("Error: No valid layers found in model")
        sys.exit(1)
    
    print(f"Found {len(layers)} layers:")
    for layer in layers:
        layer_type_name = ['Conv2D', 'Dense', 'BatchNorm', 'LayerNorm'][layer['type']]
        print(f"  {layer['name']}: {layer_type_name} {layer['weight_shape']}")
    
    # Create output directory if needed
    Path(output_path).parent.mkdir(parents=True, exist_ok=True)
    
    print(f"Writing .nwf file: {output_path}")
    
    with open(output_path, 'wb') as f:
        # Reserve space for header (will write later)
        header_pos = f.tell()
        f.write(b'\x00' * 256)
        
        # Reserve space for metadata table
        metadata_offset = f.tell()
        metadata_list = []
        for idx in range(len(layers)):
            f.write(b'\x00' * 64)
        
        # Align to 64 bytes before data
        data_offset = align_to(f.tell(), ALIGNMENT)
        f.seek(data_offset)
        
        # Write weight data and collect metadata
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
            b'NWGT',              # magic
            1,                    # version
            len(layers),          # layer_count
            total_size,           # total_size
            ALIGNMENT,            # alignment
            DTYPE_FLOAT32,        # dtype
            metadata_offset,      # metadata_offset
            data_offset,          # data_offset
            b'\x00' * 212         # reserved
        )
        f.write(header)
    
    print(f"✓ Conversion complete!")
    print(f"  Output file: {output_path}")
    print(f"  Total size: {total_size:,} bytes ({total_size / 1024 / 1024:.2f} MB)")
    print(f"  Layers: {len(layers)}")


def main():
    parser = argparse.ArgumentParser(
        description='Convert PyTorch models to NetLang Weight Format (.nwf)',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Convert MNIST model
  python convert_pytorch.py --model mnist_cnn.pth --output models/mnist.nwf
  
  # Convert with architecture hint
  python convert_pytorch.py --model resnet50.pth --output models/resnet50.nwf --arch resnet
        """
    )
    
    parser.add_argument('--model', '-m', required=True,
                        help='Path to PyTorch .pth or .pt model file')
    parser.add_argument('--output', '-o', required=True,
                        help='Output .nwf file path')
    parser.add_argument('--arch', '-a', default=None,
                        help='Architecture hint (mnist, vgg, resnet, etc.)')
    
    args = parser.parse_args()
    
    if not Path(args.model).exists():
        print(f"Error: Model file not found: {args.model}")
        sys.exit(1)
    
    convert_pytorch_to_nwf(args.model, args.output, args.arch)


if __name__ == '__main__':
    main()
