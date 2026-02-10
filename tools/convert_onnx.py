#!/usr/bin/env python3
"""
ONNX to NetLang Weight Format (.nwf) Converter

Converts ONNX .onnx model files to optimized .nwf format for NetLang compiler.
Supports Conv, Gemm (Dense), BatchNormalization layers.

Usage:
    python convert_onnx.py --model path/to/model.onnx --output weights.nwf
    
Example:
    python convert_onnx.py --model resnet50.onnx --output models/resnet50.nwf
"""

import numpy as np
import struct
import argparse
import sys
from pathlib import Path
from typing import List, Dict, Tuple

try:
    import onnx
    from onnx import numpy_helper
except ImportError:
    print("Error: ONNX not installed. Install with: pip install onnx")
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


def extract_onnx_layers(model) -> List[Dict]:
    """
    Extract layer weights from ONNX model
    
    Args:
        model: ONNX ModelProto
        
    Returns:
        List of layer dictionaries with weights, biases, types
    """
    # Build initializer lookup
    initializers = {}
    for init in model.graph.initializer:
        initializers[init.name] = numpy_helper.to_array(init)
    
    layers = []
    layer_id = 0
    
    for node in model.graph.node:
        op_type = node.op_type
        
        if op_type == 'Conv':
            # ONNX Conv weights: [out_channels, in_channels, kernel_h, kernel_w]
            # Need to transpose to NetLang: [kernel_h, kernel_w, in_channels, out_channels]
            weight_name = node.input[1]
            if weight_name not in initializers:
                print(f"Warning: Weight {weight_name} not found in initializers")
                continue
            
            weight = initializers[weight_name]
            weight_transposed = np.transpose(weight, (2, 3, 1, 0))  # [Kh, Kw, Cin, Cout]
            
            bias = None
            if len(node.input) > 2:
                bias_name = node.input[2]
                if bias_name in initializers:
                    bias = initializers[bias_name]
            
            layers.append({
                'name': node.name or f'conv_{layer_id}',
                'type': LAYER_CONV2D,
                'weight': weight_transposed,
                'weight_shape': list(weight_transposed.shape),
                'bias': bias,
                'bias_shape': [len(bias)] if bias is not None else [0]
            })
            layer_id += 1
            
        elif op_type == 'Gemm':
            # ONNX Gemm (General Matrix Multiply) = Dense layer
            # Weights: [in_features, out_features] or [out_features, in_features]
            # Check transB attribute
            weight_name = node.input[1]
            if weight_name not in initializers:
                print(f"Warning: Weight {weight_name} not found in initializers")
                continue
            
            weight = initializers[weight_name]
            
            # Check if weight needs transpose (transB attribute)
            trans_b = False
            for attr in node.attribute:
                if attr.name == 'transB':
                    trans_b = bool(attr.i)
            
            if trans_b:
                # Weight is [out, in], need [in, out]
                weight = np.transpose(weight)
            
            bias = None
            if len(node.input) > 2:
                bias_name = node.input[2]
                if bias_name in initializers:
                    bias = initializers[bias_name]
            
            layers.append({
                'name': node.name or f'dense_{layer_id}',
                'type': LAYER_DENSE,
                'weight': weight,
                'weight_shape': list(weight.shape) + [0, 0],
                'bias': bias,
                'bias_shape': [len(bias)] if bias is not None else [0]
            })
            layer_id += 1
            
        elif op_type == 'MatMul':
            # Alternative dense layer representation
            weight_name = node.input[1]
            if weight_name not in initializers:
                continue
            
            weight = initializers[weight_name]
            
            layers.append({
                'name': node.name or f'matmul_{layer_id}',
                'type': LAYER_DENSE,
                'weight': weight,
                'weight_shape': list(weight.shape) + [0, 0],
                'bias': None,
                'bias_shape': [0]
            })
            layer_id += 1
            
        elif op_type == 'BatchNormalization':
            # ONNX BN: scale (gamma), bias (beta), mean, var
            # We need scale and bias for inference
            scale_name = node.input[1]
            bias_name = node.input[2]
            
            if scale_name not in initializers or bias_name not in initializers:
                print(f"Warning: BatchNorm weights not found")
                continue
            
            scale = initializers[scale_name]
            bias = initializers[bias_name]
            
            layers.append({
                'name': node.name or f'bn_{layer_id}',
                'type': LAYER_BATCHNORM,
                'weight': scale,
                'weight_shape': [len(scale), 0, 0, 0],
                'bias': bias,
                'bias_shape': [len(bias)]
            })
            layer_id += 1
    
    return layers


def convert_onnx_to_nwf(model_path: str, output_path: str):
    """
    Convert ONNX model to .nwf format
    
    Args:
        model_path: Path to .onnx file
        output_path: Output .nwf file path
    """
    print(f"Loading ONNX model: {model_path}")
    
    try:
        model = onnx.load(model_path)
        onnx.checker.check_model(model)
    except Exception as e:
        print(f"Error loading ONNX model: {e}")
        sys.exit(1)
    
    print(f"Model IR version: {model.ir_version}")
    print(f"Model producer: {model.producer_name}")
    
    print(f"Extracting layers...")
    layers = extract_onnx_layers(model)
    
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
        description='Convert ONNX models to NetLang Weight Format (.nwf)',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Convert ONNX model
  python convert_onnx.py --model resnet50.onnx --output models/resnet50.nwf
  
  # Convert with verbose output
  python convert_onnx.py --model model.onnx --output weights.nwf
        """
    )
    
    parser.add_argument('--model', '-m', required=True,
                        help='Path to ONNX .onnx file')
    parser.add_argument('--output', '-o', required=True,
                        help='Output .nwf file path')
    
    args = parser.parse_args()
    
    if not Path(args.model).exists():
        print(f"Error: Model file not found: {args.model}")
        sys.exit(1)
    
    convert_onnx_to_nwf(args.model, args.output)


if __name__ == '__main__':
    main()
