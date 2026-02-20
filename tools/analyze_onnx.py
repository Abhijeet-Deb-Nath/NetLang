#!/usr/bin/env python3
"""Analyze ONNX model structure"""
import onnx
from onnx import numpy_helper

m = onnx.load('onnx_weight_files/lenet_mnist.onnx')

print("=== ONNX Model Structure ===\n")

# Print nodes
for node in m.graph.node:
    print(f"{node.name}: {node.op_type}")
    print(f"  Inputs: {list(node.input)}")
    print(f"  Outputs: {list(node.output)}")
    for attr in node.attribute:
        if attr.HasField('i'):
            print(f"  {attr.name} = {attr.i}")
        elif attr.HasField('f'):
            print(f"  {attr.name} = {attr.f}")
    print()

# Print weight shapes
print("\n=== Initializer Shapes ===\n")
for init in m.graph.initializer:
    arr = numpy_helper.to_array(init)
    print(f"{init.name}: {arr.shape}")
