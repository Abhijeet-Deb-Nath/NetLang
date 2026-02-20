#!/usr/bin/env python3
"""
Convert downloaded LeNet MNIST weights to .nwf format

Usage:
    python convert_lenet_weights.py lenet_mnist_model.pth models/lenet5.nwf
"""

import sys
import os
sys.path.append(os.path.dirname(__file__))

import torch
import torch.nn as nn
from convert_pytorch import extract_layers_from_model, write_nwf_file


class LeNet(nn.Module):
    """LeNet architecture matching the downloaded model"""
    def __init__(self):
        super(LeNet, self).__init__()
        self.conv1 = nn.Conv2d(1, 6, kernel_size=5, stride=1, padding=0)
        self.relu1 = nn.ReLU()
        self.pool1 = nn.MaxPool2d(kernel_size=2, stride=2)

        self.conv2 = nn.Conv2d(6, 16, kernel_size=5, stride=1, padding=0)
        self.relu2 = nn.ReLU()
        self.pool2 = nn.MaxPool2d(kernel_size=2, stride=2)

        self.fc1 = nn.Linear(256, 120)
        self.relu3 = nn.ReLU()
        self.fc2 = nn.Linear(120, 84)
        self.relu4 = nn.ReLU()
        self.fc3 = nn.Linear(84, 10)

    def forward(self, x):
        y = self.conv1(x)
        y = self.relu1(y)
        y = self.pool1(y)

        y = self.conv2(y)
        y = self.relu2(y)
        y = self.pool2(y)

        y = y.view(y.shape[0], -1)

        y = self.fc1(y)
        y = self.relu3(y)

        y = self.fc2(y)
        y = self.relu4(y)

        y = self.fc3(y)
        return y


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python convert_lenet_weights.py <input.pth> <output.nwf>")
        print("Example: python convert_lenet_weights.py lenet_mnist_model.pth models/lenet5.nwf")
        sys.exit(1)
    
    input_path = sys.argv[1]
    output_path = sys.argv[2]
    
    # Create model
    print("Creating LeNet model...")
    model = LeNet()
    model.eval()
    
    # Load state dict
    print(f"Loading weights from {input_path}...")
    state_dict = torch.load(input_path, map_location='cpu')
    model.load_state_dict(state_dict)
    
    print("Model loaded successfully!")
    print("\nArchitecture:")
    print(f"  conv1: Conv2d(1, 6, kernel=5)")
    print(f"  pool1: MaxPool2d(2, stride=2)")
    print(f"  conv2: Conv2d(6, 16, kernel=5)")
    print(f"  pool2: MaxPool2d(2, stride=2)")
    print(f"  fc1: Linear(256, 120)")
    print(f"  fc2: Linear(120, 84)")
    print(f"  fc3: Linear(84, 10)")
    
    # Extract layers
    print("\nExtracting layers...")
    layers = extract_layers_from_model(model)
    
    print(f"\nFound {len(layers)} weight layers:")
    for i, layer in enumerate(layers):
        layer_type_name = ['Conv2D', 'Dense', 'BatchNorm'][layer.layer_type]
        print(f"  {i}: {layer.name} ({layer_type_name}) shape={layer.weight_shape()}")
    
    # Create output directory if needed
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    # Convert to .nwf
    write_nwf_file(layers, output_path, verbose=True)
    
    print("\n✓ Conversion complete!")
    print(f"\nNext steps:")
    print(f"  1. Compile your network: ./build/netlang.exe examples/mnist.nlang -o build/mnist.c")
    print(f"  2. Build executable: gcc -O3 -mavx2 -I. build/mnist.c src/codegen/runtime.c src/codegen/kernels.c -o build/mnist.exe")
    print(f"  3. Run inference: ./build/mnist.exe")
