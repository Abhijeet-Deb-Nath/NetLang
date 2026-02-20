#!/usr/bin/env python3
"""
NetLang Weight Converter - ONNX to .nwf

Converts ONNX model weights (.onnx) to NetLang Weight Format (.nwf)
with optimal memory layout and 64-byte alignment for AVX2 performance.

Author: Abhijeet Deb Nath
Date: February 2026

Usage:
    python convert_onnx.py --model vgg16.onnx --output vgg16.nwf
    python convert_onnx.py --model resnet50.onnx --output resnet50.nwf --verbose
"""

import argparse
import struct
import sys
import os
from typing import List, Tuple, Dict
import numpy as np

try:
    import onnx
    from onnx import numpy_helper
except ImportError:
    print("Error: ONNX not installed. Run: pip install onnx")
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
        
        # Determine bias shape based on layer type
        if bias is None:
            if layer_type == LAYER_TYPE_CONV2D:
                bias_size = weights.shape[0]  # out_channels
            elif layer_type == LAYER_TYPE_DENSE:
                bias_size = weights.shape[0]  # out_features
            else:
                bias_size = weights.shape[0]
            self.bias = np.zeros(bias_size, dtype=np.float32)
        else:
            self.bias = bias
        
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


def extract_layers_from_onnx_model(model: onnx.ModelProto) -> List[LayerInfo]:
    """
    Extract Conv2D, Gemm (Dense), and BatchNorm layers from ONNX model.
    Handles various ONNX operator types.
    """
    layers = []
    
    # Build a map of initializers (weights and biases)
    initializers = {}
    for init in model.graph.initializer:
        initializers[init.name] = numpy_helper.to_array(init)
    
    # Track which initializers are used
    used_initializers = set()
    
    # Process each node in the graph
    for node in model.graph.node:
        if node.op_type == 'Conv':
            # Conv2D layer
            # Inputs: [input, weights, bias (optional)]
            weight_name = node.input[1]
            bias_name = node.input[2] if len(node.input) > 2 else None
            
            if weight_name in initializers:
                weights = initializers[weight_name].astype(np.float32)  # [out_ch, in_ch, kh, kw]
                bias = initializers[bias_name].astype(np.float32) if bias_name and bias_name in initializers else None
                
                layers.append(LayerInfo(node.name, LAYER_TYPE_CONV2D, weights, bias))
                used_initializers.add(weight_name)
                if bias_name:
                    used_initializers.add(bias_name)
        
        elif node.op_type == 'Gemm':
            # Dense/Linear layer (General Matrix Multiplication)
            # Y = alpha * A * B + beta * C
            # Typically: output = input @ weights + bias
            # Inputs: [input, weights, bias (optional)]
            weight_name = node.input[1]
            bias_name = node.input[2] if len(node.input) > 2 else None
            
            if weight_name in initializers:
                weights = initializers[weight_name].astype(np.float32)
                
                # Check for transposition attributes
                transA = False
                transB = False
                for attr in node.attribute:
                    if attr.name == 'transA':
                        transA = attr.i == 1
                    elif attr.name == 'transB':
                        transB = attr.i == 1
                
                # ONNX Gemm typically has weights as [in_features, out_features]
                # We need [out_features, in_features]
                if not transB:
                    weights = weights.T
                
                bias = initializers[bias_name].astype(np.float32) if bias_name and bias_name in initializers else None
                
                layers.append(LayerInfo(node.name, LAYER_TYPE_DENSE, weights, bias))
                used_initializers.add(weight_name)
                if bias_name:
                    used_initializers.add(bias_name)
        
        elif node.op_type == 'MatMul':
            # Matrix multiplication (Dense without bias)
            weight_name = node.input[1]
            
            if weight_name in initializers:
                weights = initializers[weight_name].astype(np.float32)
                
                # MatMul output shape: [batch, in] @ [in, out] = [batch, out]
                # We need [out, in] for our format
                if len(weights.shape) == 2:
                    weights = weights.T
                
                layers.append(LayerInfo(node.name, LAYER_TYPE_DENSE, weights, None))
                used_initializers.add(weight_name)
        
        elif node.op_type == 'BatchNormalization':
            # BatchNorm layer
            # Inputs: [input, scale (gamma), bias (beta), mean, variance]
            if len(node.input) >= 3:
                scale_name = node.input[1]
                bias_name = node.input[2]
                
                if scale_name in initializers and bias_name in initializers:
                    gamma = initializers[scale_name].astype(np.float32)
                    beta = initializers[bias_name].astype(np.float32)
                    
                    layers.append(LayerInfo(node.name, LAYER_TYPE_BATCHNORM, gamma, beta))
                    used_initializers.add(scale_name)
                    used_initializers.add(bias_name)
    
    # Check for unused initializers (might be additional weights we missed)
    unused = set(initializers.keys()) - used_initializers
    if unused:
        print(f"Warning: {len(unused)} unused initializers found (might be moving stats, etc.)")
    
    return layers


def load_onnx_model(model_path: str) -> onnx.ModelProto:
    """Load ONNX model from file"""
    if not os.path.exists(model_path):
        raise FileNotFoundError(f"Model file not found: {model_path}")
    
    print(f"Loading ONNX model: {model_path}")
    
    try:
        model = onnx.load(model_path)
        
        # Validate model
        try:
            onnx.checker.check_model(model)
            print("✓ ONNX model is valid")
        except Exception as e:
            print(f"Warning: ONNX model validation failed: {e}")
            print("Continuing anyway...")
        
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
        # Metadata entry size: 4+4+8+8+16+8+8+4+4 = 64 bytes per layer
        METADATA_ENTRY_SIZE = 64
        data_offset = metadata_offset + (layer_count * METADATA_ENTRY_SIZE)
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
        # total_size is at offset 12 (4 magic + 4 version + 4 layer_count = 12)
        f.seek(12)
        f.write(struct.pack('<Q', total_size))
    
    # Print summary
    print(f"\n✓ Conversion complete!")
    print(f"  Output file: {output_path}")
    print(f"  Total size: {total_size:,} bytes ({total_size / (1024**2):.2f} MB)")
    print(f"  Layers: {len(layers)}")
    print(f"  Alignment: {alignment} bytes")


def main():
    parser = argparse.ArgumentParser(
        description="Convert ONNX model weights to NetLang .nwf format",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python convert_onnx.py --model vgg16.onnx --output vgg16.nwf
  python convert_onnx.py --model resnet50.onnx --output resnet50.nwf --verbose

Notes:
  - Extracts Conv, Gemm (Dense), MatMul, and BatchNormalization operations
  - Optimizes Dense layer weight layout for cache efficiency
  - Ensures 64-byte alignment for AVX2 performance
  - Validates ONNX model before conversion
        """
    )
    
    parser.add_argument('--model', required=True, help='Input ONNX model file (.onnx)')
    parser.add_argument('--output', required=True, help='Output .nwf file')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    # Load model
    model = load_onnx_model(args.model)
    
    # Extract layers
    layers = extract_layers_from_onnx_model(model)
    
    if len(layers) == 0:
        print("Error: No Conv, Gemm, or BatchNorm layers found in model")
        print("Make sure the model contains weight parameters (not just the graph structure)")
        sys.exit(1)
    
    print(f"\nFound {len(layers)} layers:")
    for i, layer in enumerate(layers):
        layer_type_name = ['Conv2D', 'Dense', 'BatchNorm'][layer.layer_type]
        print(f"  {i}: {layer.name} ({layer_type_name}) {layer.weight_shape()}")
    
    # Convert to .nwf
    write_nwf_file(layers, args.output, args.verbose)


if __name__ == '__main__':
    main()
