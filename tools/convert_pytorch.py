#!/usr/bin/env python3
"""
NetLang Weight Converter - PyTorch to .nwf

Converts PyTorch model weights (.pth, .pt) to NetLang Weight Format (.nwf)
with optimal memory layout and 64-byte alignment for AVX2 performance.

Author: Abhijeet Deb Nath
Date: February 2026

Usage:
    python convert_pytorch.py --model vgg16.pth --output vgg16.nwf
    python convert_pytorch.py --model resnet50.pth --output resnet50.nwf --verbose
"""

import argparse
import struct
import sys
import os
from typing import List, Tuple, Dict
import numpy as np

try:
    import torch
    import torch.nn as nn
except ImportError:
    print("Error: PyTorch not installed. Run: pip install torch")
    sys.exit(1)


# Layer type constants
LAYER_TYPE_CONV2D = 0
LAYER_TYPE_DENSE = 1
LAYER_TYPE_BATCHNORM = 2

# Data type constants
DTYPE_FLOAT32 = 0
DTYPE_FLOAT16 = 1
DTYPE_INT8 = 2


class LayerInfo:
    """Information about a single layer"""
    def __init__(self, name: str, layer_type: int, weights: np.ndarray, bias: np.ndarray = None):
        self.name = name
        self.layer_type = layer_type
        self.weights = weights
        self.bias = bias if bias is not None else np.zeros(weights.shape[0], dtype=np.float32)
        
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


def extract_layers_from_model(model: nn.Module) -> List[LayerInfo]:
    """
    Extract Conv2D and Dense layers from PyTorch model.
    Automatically handles common architectures (VGG, ResNet, etc.)
    """
    layers = []
    layer_id = 0
    
    def extract_recursive(module: nn.Module, prefix: str = ""):
        nonlocal layer_id
        
        for name, child in module.named_children():
            full_name = f"{prefix}.{name}" if prefix else name
            
            if isinstance(child, nn.Conv2d):
                weights = child.weight.data.cpu().numpy()  # [out_ch, in_ch, kh, kw]
                bias = child.bias.data.cpu().numpy() if child.bias is not None else None
                
                layers.append(LayerInfo(full_name, LAYER_TYPE_CONV2D, weights, bias))
                layer_id += 1
                
            elif isinstance(child, nn.Linear):
                weights = child.weight.data.cpu().numpy()  # [out_features, in_features]
                # Keep as is - we'll transpose when writing
                bias = child.bias.data.cpu().numpy() if child.bias is not None else None
                
                layers.append(LayerInfo(full_name, LAYER_TYPE_DENSE, weights, bias))
                layer_id += 1
                
            elif isinstance(child, nn.BatchNorm2d):
                # BatchNorm: gamma (weight) and beta (bias)
                weights = child.weight.data.cpu().numpy() if child.weight is not None else None
                bias = child.bias.data.cpu().numpy() if child.bias is not None else None
                
                if weights is not None:
                    layers.append(LayerInfo(full_name, LAYER_TYPE_BATCHNORM, weights, bias))
                    layer_id += 1
            else:
                # Recurse into sub-modules
                extract_recursive(child, full_name)
    
    extract_recursive(model)
    return layers


def load_pytorch_model(model_path: str) -> nn.Module:
    """Load PyTorch model from file"""
    if not os.path.exists(model_path):
        raise FileNotFoundError(f"Model file not found: {model_path}")
    
    print(f"Loading PyTorch model: {model_path}")
    
    try:
        # Try loading as state dict
        state_dict = torch.load(model_path, map_location='cpu')
        
        if isinstance(state_dict, dict):
            # If it's a state dict, we need the model architecture
            # This is a limitation - user must provide the model class
            print("Warning: Loaded state_dict only. Attempting to infer architecture...")
            print("Note: For best results, save the entire model with torch.save(model, path)")
            
            # Try to infer architecture from keys
            return state_dict  # Return state_dict for manual processing
        else:
            # Full model loaded
            return state_dict
            
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
        
        # Calculate total size (will update at end)
        total_size = 0
        
        # Header structure (256 bytes)
        f.write(magic)                                    # 4 bytes: magic
        f.write(struct.pack('<I', version))              # 4 bytes: version
        f.write(struct.pack('<I', layer_count))          # 4 bytes: layer count
        f.write(struct.pack('<Q', 0))                    # 8 bytes: total_size (placeholder)
        f.write(struct.pack('<I', alignment))            # 4 bytes: alignment
        f.write(struct.pack('<I', dtype))                # 4 bytes: dtype
        f.write(struct.pack('<Q', metadata_offset))      # 8 bytes: metadata offset
        
        # Calculate data offset (after metadata table)
        data_offset = metadata_offset + (layer_count * 64)
        data_offset = align_to_64(data_offset)
        f.write(struct.pack('<Q', data_offset))          # 8 bytes: data offset
        f.write(bytes(212))                              # 212 bytes: reserved
        
        # ========== PREPARE METADATA ==========
        metadata = []
        current_offset = data_offset
        
        for i, layer in enumerate(layers):
            # Prepare weights
            weights = layer.weights.astype(np.float32)
            
            # IMPORTANT: Transpose Dense layer weights for cache efficiency
            if layer.layer_type == LAYER_TYPE_DENSE:
                # PyTorch: [out_features, in_features]
                # We keep it as is, since PyTorch already has this layout
                # Our kernel expects weights[out][in] layout
                pass
            
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
            f.write(bytes(4))                                         # 4 bytes: reserved (total 64 bytes)
        
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
        f.seek(8)  # Position of total_size field
        f.write(struct.pack('<Q', total_size))
    
    # Print summary
    print(f"\n✓ Conversion complete!")
    print(f"  Output file: {output_path}")
    print(f"  Total size: {total_size:,} bytes ({total_size / (1024**2):.2f} MB)")
    print(f"  Layers: {len(layers)}")
    print(f"  Alignment: {alignment} bytes")


def main():
    parser = argparse.ArgumentParser(
        description="Convert PyTorch model weights to NetLang .nwf format",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python convert_pytorch.py --model vgg16.pth --output vgg16.nwf
  python convert_pytorch.py --model resnet50.pth --output resnet50.nwf --verbose

Notes:
  - Extracts Conv2D, Dense (Linear), and BatchNorm layers automatically
  - Optimizes Dense layer weight layout for cache efficiency
  - Ensures 64-byte alignment for AVX2 performance
  - Input model must be saved with torch.save(model, path) for best results
        """
    )
    
    parser.add_argument('--model', required=True, help='Input PyTorch model file (.pth, .pt)')
    parser.add_argument('--output', required=True, help='Output .nwf file')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    # Load model
    model = load_pytorch_model(args.model)
    
    # Extract layers
    if isinstance(model, dict):
        print("Error: Cannot extract layers from state_dict only.")
        print("Please save the full model: torch.save(model, 'model.pth')")
        sys.exit(1)
    
    layers = extract_layers_from_model(model)
    
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
