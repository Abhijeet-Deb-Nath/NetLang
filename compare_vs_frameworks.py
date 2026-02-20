#!/usr/bin/env python3
"""
Comprehensive Inference Speed Benchmark
Compares NetLang vs Production ML Frameworks

Frameworks tested:
- NetLang (custom compiler)
- OpenVINO (Intel optimizer)
- ONNX Runtime (Microsoft runtime)
- TVM (Apache ML compiler) [if available]
- PyTorch Compiled (torch.compile optimization)
- PyTorch Eager (baseline reference)

Author: NetLang Compiler Project
Date: February 2026
"""

import subprocess
import time
import numpy as np
from pathlib import Path

# Framework availability
FRAMEWORKS = {
    'netlang': Path('bin/test_lenet5.exe').exists(),
    'onnxruntime': False,
    'openvino': False,
    'tvm': False,
    'pytorch': False
}

# Check Python frameworks
try:
    import onnxruntime
    FRAMEWORKS['onnxruntime'] = True
except ImportError:
    pass

try:
    import openvino
    FRAMEWORKS['openvino'] = True
except ImportError:
    pass

try:
    import tvm
    FRAMEWORKS['tvm'] = True
except ImportError:
    pass

try:
    import torch
    FRAMEWORKS['pytorch'] = True
except ImportError:
    pass


def load_test_image(filepath):
    """Load preprocessed MNIST test image"""
    img = np.fromfile(filepath, dtype=np.float32)
    return img.reshape(1, 1, 28, 28)


def benchmark_netlang(test_files, n_warmup=10, n_runs=100):
    """Benchmark NetLang compiled executable (FAIR - uses internal timing)"""
    print("\n[1/6] NetLang Compiler")
    print("=" * 60)
    
    if not FRAMEWORKS['netlang']:
        print("❌ NetLang executable not found (bin/test_lenet5.exe)")
        return None
    
    times = []
    correct = 0
    
    print("Note: Using internal timing from C code (fair comparison)")
    
    # Warmup
    for _ in range(n_warmup):
        subprocess.run(['bin\\test_lenet5.exe', test_files[0]], 
                      capture_output=True, text=True)
    
    # Benchmark - extract internal timing from C code
    for test_file in test_files:
        label = int(test_file.stem.split('_')[-1])
        
        result = subprocess.run(['bin\\test_lenet5.exe', str(test_file)],
                              capture_output=True, text=True)
        
        # Parse prediction and internal timing
        internal_time = None
        for line in result.stdout.split('\n'):
            if 'RESULT:' in line:
                pred = int(line.split(':')[1].strip())
                if pred == label:
                    correct += 1
            
            # Extract internal inference time: "OK (4.00 ms)"
            if 'OK (' in line and 'ms)' in line:
                try:
                    time_str = line.split('(')[1].split('ms')[0].strip()
                    internal_time = float(time_str)
                except:
                    pass
        
        if internal_time:
            times.append(internal_time)
    
    if not times:
        print("⚠ Warning: Could not extract internal timing, falling back to subprocess timing")
        # Fallback: re-run with subprocess timing
        times = []
        for test_file in test_files:
            start = time.perf_counter()
            subprocess.run(['bin\\test_lenet5.exe', str(test_file)],
                          capture_output=True, text=True)
            times.append((time.perf_counter() - start) * 1000)
    
    avg_time = np.mean(times)
    std_time = np.std(times)
    accuracy = 100 * correct / len(test_files)
    
    print(f"✓ Inference time: {avg_time:.2f} ± {std_time:.2f} ms (internal timing)")
    print(f"✓ Accuracy: {accuracy:.1f}% ({correct}/{len(test_files)})")
    
    return {'mean': avg_time, 'std': std_time, 'accuracy': accuracy}


def benchmark_onnxruntime(test_files, n_warmup=10, n_runs=100):
    """Benchmark ONNX Runtime"""
    print("\n[2/6] ONNX Runtime")
    print("=" * 60)
    
    if not FRAMEWORKS['onnxruntime']:
        print("❌ ONNX Runtime not installed (pip install onnxruntime)")
        return None
    
    import onnxruntime as ort
    
    # Load model
    session = ort.InferenceSession("onnx_weight_files/lenet_mnist.onnx")
    input_name = session.get_inputs()[0].name
    
    times = []
    correct = 0
    
    # Warmup
    dummy = load_test_image(test_files[0])
    for _ in range(n_warmup):
        session.run(None, {input_name: dummy})
    
    # Benchmark
    for test_file in test_files:
        label = int(test_file.stem.split('_')[-1])
        img = load_test_image(test_file)
        
        start = time.perf_counter()
        output = session.run(None, {input_name: img})
        elapsed = (time.perf_counter() - start) * 1000
        
        pred = np.argmax(output[0])
        if pred == label:
            correct += 1
        
        times.append(elapsed)
    
    avg_time = np.mean(times)
    std_time = np.std(times)
    accuracy = 100 * correct / len(test_files)
    
    print(f"✓ Inference time: {avg_time:.2f} ± {std_time:.2f} ms")
    print(f"✓ Accuracy: {accuracy:.1f}% ({correct}/{len(test_files)})")
    
    return {'mean': avg_time, 'std': std_time, 'accuracy': accuracy}


def benchmark_openvino(test_files, n_warmup=10, n_runs=100):
    """Benchmark OpenVINO"""
    print("\n[3/6] OpenVINO (Intel Optimizer)")
    print("=" * 60)
    
    if not FRAMEWORKS['openvino']:
        print("❌ OpenVINO not installed (pip install openvino)")
        return None
    
    import openvino as ov
    
    # Load and compile model
    core = ov.Core()
    model = core.read_model("onnx_weight_files/lenet_mnist.onnx")
    compiled_model = core.compile_model(model, "CPU")
    infer_request = compiled_model.create_infer_request()
    
    times = []
    correct = 0
    
    # Warmup
    dummy = load_test_image(test_files[0])
    for _ in range(n_warmup):
        infer_request.infer({0: dummy})
    
    # Benchmark
    for test_file in test_files:
        label = int(test_file.stem.split('_')[-1])
        img = load_test_image(test_file)
        
        start = time.perf_counter()
        infer_request.infer({0: img})
        output = infer_request.get_output_tensor(0).data
        elapsed = (time.perf_counter() - start) * 1000
        
        pred = np.argmax(output)
        if pred == label:
            correct += 1
        
        times.append(elapsed)
    
    avg_time = np.mean(times)
    std_time = np.std(times)
    accuracy = 100 * correct / len(test_files)
    
    print(f"✓ Inference time: {avg_time:.2f} ± {std_time:.2f} ms")
    print(f"✓ Accuracy: {accuracy:.1f}% ({correct}/{len(test_files)})")
    
    return {'mean': avg_time, 'std': std_time, 'accuracy': accuracy}


def benchmark_tvm(test_files, n_warmup=10, n_runs=100):
    """Benchmark TVM"""
    print("\n[4/6] TVM (Apache ML Compiler)")
    print("=" * 60)
    
    if not FRAMEWORKS['tvm']:
        print("❌ TVM not installed (pip install apache-tvm)")
        return None
    
    try:
        import tvm
        from tvm import relay
        import onnx
        
        # Load ONNX model
        onnx_model = onnx.load("onnx_weight_files/lenet_mnist.onnx")
        
        # Convert to Relay IR
        shape_dict = {"input": (1, 1, 28, 28)}
        mod, params = relay.frontend.from_onnx(onnx_model, shape_dict)
        
        # Compile for CPU
        target = "llvm -mcpu=core-avx2"
        with tvm.transform.PassContext(opt_level=3):
            lib = relay.build(mod, target=target, params=params)
        
        # Create runtime
        from tvm.contrib import graph_executor
        dev = tvm.cpu(0)
        module = graph_executor.GraphModule(lib["default"](dev))
        
        times = []
        correct = 0
        
        # Warmup
        dummy = load_test_image(test_files[0])
        for _ in range(n_warmup):
            module.set_input("input", tvm.nd.array(dummy, dev))
            module.run()
        
        # Benchmark
        for test_file in test_files:
            label = int(test_file.stem.split('_')[-1])
            img = load_test_image(test_file)
            
            module.set_input("input", tvm.nd.array(img, dev))
            
            start = time.perf_counter()
            module.run()
            output = module.get_output(0).numpy()
            elapsed = (time.perf_counter() - start) * 1000
            
            pred = np.argmax(output)
            if pred == label:
                correct += 1
            
            times.append(elapsed)
        
        avg_time = np.mean(times)
        std_time = np.std(times)
        accuracy = 100 * correct / len(test_files)
        
        print(f"✓ Inference time: {avg_time:.2f} ± {std_time:.2f} ms")
        print(f"✓ Accuracy: {accuracy:.1f}% ({correct}/{len(test_files)})")
        
        return {'mean': avg_time, 'std': std_time, 'accuracy': accuracy}
    
    except Exception as e:
        print(f"❌ TVM benchmark failed: {e}")
        return None


def load_pytorch_model_from_onnx():
    """Load PyTorch model with weights from ONNX"""
    import torch
    import torch.nn as nn
    import onnx
    from onnx import numpy_helper
    
    # Define model matching ONNX architecture
    class LeNet5(nn.Module):
        def __init__(self):
            super().__init__()
            self.conv1 = nn.Conv2d(1, 32, 5)
            self.conv2 = nn.Conv2d(32, 32, 5)
            self.fc1 = nn.Linear(512, 100)
            self.fc2 = nn.Linear(100, 100)
            self.fc3 = nn.Linear(100, 10)
            self.pool = nn.MaxPool2d(2, 2)
            
        def forward(self, x):
            x = torch.relu(self.conv1(x))
            x = self.pool(x)
            x = torch.relu(self.conv2(x))
            x = self.pool(x)
            x = x.view(-1, 512)
            x = torch.relu(self.fc1(x))
            x = torch.relu(self.fc2(x))
            x = self.fc3(x)
            return x
    
    model = LeNet5()
    
    # Load weights from ONNX
    onnx_model = onnx.load("onnx_weight_files/lenet_mnist.onnx")
    weights = {init.name: numpy_helper.to_array(init) for init in onnx_model.graph.initializer}
    
    # Map ONNX weights to PyTorch
    with torch.no_grad():
        if '/0/Conv.weight' in weights:
            model.conv1.weight.copy_(torch.from_numpy(weights['/0/Conv.weight']))
            model.conv1.bias.copy_(torch.from_numpy(weights['/0/Conv.bias']))
        if '/3/Conv.weight' in weights:
            model.conv2.weight.copy_(torch.from_numpy(weights['/3/Conv.weight']))
            model.conv2.bias.copy_(torch.from_numpy(weights['/3/Conv.bias']))
        if '/7/Gemm.weight' in weights:
            model.fc1.weight.copy_(torch.from_numpy(weights['/7/Gemm.weight']))
            model.fc1.bias.copy_(torch.from_numpy(weights['/7/Gemm.bias']))
        if '/9/Gemm.weight' in weights:
            model.fc2.weight.copy_(torch.from_numpy(weights['/9/Gemm.weight']))
            model.fc2.bias.copy_(torch.from_numpy(weights['/9/Gemm.bias']))
        if '/11/Gemm.weight' in weights:
            model.fc3.weight.copy_(torch.from_numpy(weights['/11/Gemm.weight']))
            model.fc3.bias.copy_(torch.from_numpy(weights['/11/Gemm.bias']))
    
    model.eval()
    return model


def benchmark_pytorch_compiled(test_files, n_warmup=10, n_runs=100):
    """Benchmark PyTorch with torch.compile() optimization"""
    print("\n[5/6] PyTorch (Compiled - Optimized)")
    print("=" * 60)
    
    if not FRAMEWORKS['pytorch']:
        print("❌ PyTorch not installed (pip install torch)")
        return None
    
    try:
        import torch
        
        # Load model
        model = load_pytorch_model_from_onnx()
        
        # Compile with torch.compile for optimization
        print("Compiling model with torch.compile()...")
        compiled_model = torch.compile(model, mode='max-autotune')
        
        times = []
        correct = 0
        
        # Warmup (important for compiled models)
        print("Warming up compiled model...")
        with torch.no_grad():
            dummy = torch.from_numpy(load_test_image(test_files[0]))
            for _ in range(n_warmup):
                compiled_model(dummy)
        
        # Benchmark
        with torch.no_grad():
            for test_file in test_files:
                label = int(test_file.stem.split('_')[-1])
                img = torch.from_numpy(load_test_image(test_file))
                
                start = time.perf_counter()
                output = compiled_model(img)
                elapsed = (time.perf_counter() - start) * 1000
                
                pred = torch.argmax(output).item()
                if pred == label:
                    correct += 1
                
                times.append(elapsed)
        
        avg_time = np.mean(times)
        std_time = np.std(times)
        accuracy = 100 * correct / len(test_files)
        
        print(f"✓ Inference time: {avg_time:.2f} ± {std_time:.2f} ms")
        print(f"✓ Accuracy: {accuracy:.1f}% ({correct}/{len(test_files)})")
        
        return {'mean': avg_time, 'std': std_time, 'accuracy': accuracy}
    
    except Exception as e:
        print(f"❌ PyTorch compiled benchmark failed: {e}")
        return None


def benchmark_pytorch_eager(test_files, n_warmup=10, n_runs=100):
    """Benchmark PyTorch eager mode (baseline)"""
    print("\n[6/6] PyTorch (Eager - Baseline)")
    print("=" * 60)
    
    if not FRAMEWORKS['pytorch']:
        print("❌ PyTorch not installed (pip install torch)")
        return None
    
    try:
        import torch
        
        # Load model
        model = load_pytorch_model_from_onnx()
        
        times = []
        correct = 0
        
        # Warmup
        with torch.no_grad():
            dummy = torch.from_numpy(load_test_image(test_files[0]))
            for _ in range(n_warmup):
                model(dummy)
        
        # Benchmark
        with torch.no_grad():
            for test_file in test_files:
                label = int(test_file.stem.split('_')[-1])
                img = torch.from_numpy(load_test_image(test_file))
                
                start = time.perf_counter()
                output = model(img)
                elapsed = (time.perf_counter() - start) * 1000
                
                pred = torch.argmax(output).item()
                if pred == label:
                    correct += 1
                
                times.append(elapsed)
        
        avg_time = np.mean(times)
        std_time = np.std(times)
        accuracy = 100 * correct / len(test_files)
        
        print(f"✓ Inference time: {avg_time:.2f} ± {std_time:.2f} ms")
        print(f"✓ Accuracy: {accuracy:.1f}% ({correct}/{len(test_files)})")
        
        return {'mean': avg_time, 'std': std_time, 'accuracy': accuracy}
    
    except Exception as e:
        print(f"❌ PyTorch eager benchmark failed: {e}")
        return None


def print_summary(results):
    """Print comparison summary"""
    print("\n" + "=" * 60)
    print("BENCHMARK SUMMARY")
    print("=" * 60)
    print(f"{'Framework':<20} {'Time (ms)':<15} {'Accuracy':<12} {'Status'}")
    print("-" * 60)
    
    sorted_results = sorted(
        [(k, v) for k, v in results.items() if v is not None],
        key=lambda x: x[1]['mean']
    )
    
    for framework, data in sorted_results:
        time_str = f"{data['mean']:.2f} ± {data['std']:.2f}"
        acc_str = f"{data['accuracy']:.1f}%"
        status = "✓" if data['accuracy'] > 95 else "⚠"
        print(f"{framework:<20} {time_str:<15} {acc_str:<12} {status}")
    
    if sorted_results:
        fastest = sorted_results[0]
        print("\n" + "=" * 60)
        print(f"🏆 Fastest: {fastest[0]} ({fastest[1]['mean']:.2f} ms)")
        
        if 'netlang' in results and results['netlang'] is not None:
            netlang_time = results['netlang']['mean']
            for framework, data in sorted_results:
                if framework != 'netlang':
                    speedup = data['mean'] / netlang_time
                    print(f"   NetLang is {speedup:.2f}x vs {framework}")


def main():
    print("=" * 60)
    print("NetLang Inference Speed Benchmark")
    print("=" * 60)
    print(f"\nModel: LeNet5 (ONNX)")
    print(f"Test data: MNIST 28×28")
    print(f"CPU: Intel (AVX2 support)")
    
    # Get test files
    test_dir = Path('test_data/preprocessed_28x28')
    test_files = sorted(list(test_dir.glob('mnist_*.bin')))[:100]
    
    print(f"Test images: {len(test_files)}")
    print(f"\nFrameworks available:")
    for name, available in FRAMEWORKS.items():
        status = "✓" if available else "✗"
        print(f"  {status} {name}")
    
    # Run benchmarks
    results = {}
    results['netlang'] = benchmark_netlang(test_files)
    results['onnxruntime'] = benchmark_onnxruntime(test_files)
    results['openvino'] = benchmark_openvino(test_files)
    results['tvm'] = benchmark_tvm(test_files)
    results['pytorch_compiled'] = benchmark_pytorch_compiled(test_files)
    results['pytorch_eager'] = benchmark_pytorch_eager(test_files)
    
    # Print results
    print_summary(results)


if __name__ == '__main__':
    main()
