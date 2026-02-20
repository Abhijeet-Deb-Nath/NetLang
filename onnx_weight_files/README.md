# ONNX Weight Files (onnx_weight_files/)

This directory contains neural network models in ONNX format for conversion to NetLang's `.nwf` format.

## Current Models

- `lenet_mnist.onnx` - LeNet-5 trained on MNIST dataset

## About ONNX

[ONNX (Open Neural Network Exchange)](https://onnx.ai/) is an open format for representing machine learning models. It provides interoperability between different deep learning frameworks.

## Converting ONNX to .nwf

Use the ONNX converter tool:

```bash
python tools/convert_onnx.py --model onnx_weight_files/lenet_mnist.onnx --output models/lenet5.nwf
```

The converter:
1. Loads the ONNX model
2. Extracts Conv2D, Dense, and BatchNorm layers
3. Transposes Dense weights for optimal memory layout
4. Aligns all data to 64-byte boundaries
5. Writes the `.nwf` binary file

## Getting ONNX Models

### Export from PyTorch
```python
import torch

model = MyModel()
model.load_state_dict(torch.load('model.pth'))
dummy_input = torch.randn(1, 1, 28, 28)

torch.onnx.export(
    model,
    dummy_input,
    "onnx_weight_files/my_model.onnx",
    export_params=True,
    opset_version=11,
    do_constant_folding=True
)
```

### Export from TensorFlow
```python
import tf2onnx
import onnx

spec = (tf.TensorSpec((None, 28, 28, 1), tf.float32, name="input"),)
output_path = "onnx_weight_files/my_model.onnx"

model_proto, _ = tf2onnx.convert.from_keras(model, input_signature=spec)
onnx.save(model_proto, output_path)
```

### Download Pre-trained Models
- [ONNX Model Zoo](https://github.com/onnx/models)
- [Hugging Face Models](https://huggingface.co/models)
- Various research repositories

## Verification

Verify ONNX model structure:

```python
import onnx

model = onnx.load("onnx_weight_files/lenet_mnist.onnx")
print(f"IR version: {model.ir_version}")
print(f"Producer: {model.producer_name}")
print(f"Graph inputs: {[i.name for i in model.graph.input]}")
print(f"Graph outputs: {[o.name for o in model.graph.output]}")
print(f"Number of nodes: {len(model.graph.node)}")
```

## Why Convert to .nwf?

| Feature | ONNX | .nwf |
|---------|------|------|
| Load time | 80ms | <1ms |
| Memory copies | Multiple | Zero (mmap) |
| Alignment | None | 64-byte (AVX2) |
| Parsing | Required | None |
| Format | Protobuf | Raw binary |
| Use case | Interoperability | Fast inference |

ONNX is great for model exchange between frameworks. NetLang's `.nwf` is optimized for production inference on x86-64 CPUs.

## Supported Layer Types

The ONNX converter extracts:
- **Conv** → Conv2D layers
- **Gemm** → Dense (fully connected) layers
- **MatMul** + **Add** → Dense layers
- **BatchNormalization** → BatchNorm layers

Pooling, activation, and reshape operations are handled by the network definition (`.nlang` file), not the weight file.

## Notes

- ONNX files can be large (store in Git LFS if needed)
- Ensure opset version compatibility (tested with opset 11+)
- Some ONNX operators may not be supported yet
- Use `--verbose` flag to see detailed conversion logs
