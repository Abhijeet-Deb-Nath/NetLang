#!/usr/bin/env python3
"""Compare NetLang inference with ONNX Runtime"""
import numpy as np
import struct
import sys

# Load test data
test_file = sys.argv[1] if len(sys.argv) > 1 else 'test_data/preprocessed/mnist_0000_label_7.bin'
data = np.fromfile(test_file, dtype=np.float32)
print(f"Loaded {test_file}: {data.shape}")
print(f"First 5 values: {data[:5]}")
print(f"Non-zero count: {np.count_nonzero(data)}")
print(f"Min: {data.min():.4f}, Max: {data.max():.4f}")

# Reshape for ONNX (NCHW: [1, 1, 28, 28])
# Our data is HWC [28, 28, 1], need to convert
data_hwc = data.reshape(28, 28, 1)
data_chw = np.transpose(data_hwc, (2, 0, 1))  # HWC -> CHW
data_nchw = data_chw.reshape(1, 1, 28, 28)
print(f"Reshaped to NCHW: {data_nchw.shape}")

# Run ONNX inference
try:
    import onnxruntime as ort
    
    sess = ort.InferenceSession('onnx_weight_files/lenet_mnist.onnx')
    input_name = sess.get_inputs()[0].name
    
    outputs = sess.run(None, {input_name: data_nchw.astype(np.float32)})
    probs = outputs[0][0]
    
    # Apply softmax if needed
    if probs.max() > 1.0:
        probs = np.exp(probs) / np.sum(np.exp(probs))
    
    print("\n=== ONNX Runtime Prediction ===")
    print(f"Predicted class: {np.argmax(probs)}")
    print(f"Confidence: {probs.max():.2%}")
    print("\nProbabilities:")
    for i, p in enumerate(probs):
        print(f"  Class {i}: {p:.4f}")
        
except ImportError:
    print("\nonnxruntime not installed. Run: pip install onnxruntime")
