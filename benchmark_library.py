#!/usr/bin/env python3
"""
Fair benchmark using NetLang as a LIBRARY (not executable)

Loads netlang_lenet5.dll once, then calls inference function directly.
This matches ONNX Runtime's architecture (library vs subprocess).

Author: NetLang Compiler Project  
Date: February 2026
"""

import ctypes
import numpy as np
import time
from pathlib import Path

class NetLangLeNet5:
    """Python wrapper for NetLang LeNet5 DLL"""
    
    def __init__(self, dll_path="bin/netlang_lenet5.dll"):
        # Load DLL
        self.dll = ctypes.CDLL(dll_path)
        
        # Setup function signatures
        self.dll.network_init.restype = ctypes.c_void_p
        self.dll.network_init.argtypes = []
        
        self.dll.network_infer.restype = None
        self.dll.network_infer.argtypes = [
            ctypes.c_void_p,  # NetworkState*
            ctypes.POINTER(ctypes.c_float),  # input
            ctypes.POINTER(ctypes.c_float)   # output
        ]
        
        self.dll.network_cleanup.restype = None
        self.dll.network_cleanup.argtypes = [ctypes.c_void_p]
        
        # Initialize network (load weights, allocate tensors)
        print("Initializing NetLang network...")
        self.net = self.dll.network_init()
        if not self.net:
            raise RuntimeError("Failed to initialize network")
        print(f"✓ Network initialized (pointer: {hex(self.net)})")
    
    def infer(self, input_data):
        """Run inference on input data (784 floats for MNIST)"""
        # Prepare input/output buffers
        input_array = input_data.astype(np.float32).flatten()
        output_array = np.zeros(10, dtype=np.float32)
        
        input_ptr = input_array.ctypes.data_as(ctypes.POINTER(ctypes.c_float))
        output_ptr = output_array.ctypes.data_as(ctypes.POINTER(ctypes.c_float))
        
        # Call C function directly (no process spawn!)
        self.dll.network_infer(self.net, input_ptr, output_ptr)
        
        return output_array
    
    def __del__(self):
        if hasattr(self, 'net') and self.net:
            self.dll.network_cleanup(self.net)


def benchmark_netlang_library(test_files, n_warmup=10):
    """Benchmark NetLang as library (fair comparison)"""
    
    print("=" * 70)
    print("NetLang Library Benchmark (FAIR)")
    print("=" * 70)
    print()
    
    # Load model ONCE (like ONNX Runtime does)
    model = NetLangLeNet5()
    print()
    
    times = []
    correct = 0
    
    # Warmup
    dummy = np.fromfile(test_files[0], dtype=np.float32)
    for _ in range(n_warmup):
        model.infer(dummy)
    
    print(f"Running {len(test_files)} inferences...")
    
    # Benchmark  
    for i, test_file in enumerate(test_files):
        label = int(test_file.stem.split('_')[-1])
        img = np.fromfile(test_file, dtype=np.float32)
        
        # Time ONLY the inference (no process spawn!)
        start = time.perf_counter()
        output = model.infer(img)
        elapsed = (time.perf_counter() - start) * 1000
        
        pred = np.argmax(output)
        if pred == label:
            correct += 1
        
        times.append(elapsed)
        
        if (i + 1) % 20 == 0:
            print(f"  [{i+1}/{len(test_files)}] Avg: {np.mean(times):.2f} ms")
    
    avg_time = np.mean(times)
    std_time = np.std(times)
    accuracy = 100 * correct / len(test_files)
    
    print()
    print(f"✓ Inference time: {avg_time:.2f} ± {std_time:.2f} ms")
    print(f"✓ Accuracy: {accuracy:.1f}% ({correct}/{len(test_files)})")
    
    return {'mean': avg_time, 'std': std_time, 'accuracy': accuracy}


def benchmark_onnxruntime(test_files, n_warmup=10):
    """ONNX Runtime benchmark"""
    import onnxruntime as ort
    
    print()
    print("=" * 70)
    print("ONNX Runtime Benchmark")
    print("=" * 70)
    print()
    
    session = ort.InferenceSession("onnx_weight_files/lenet_mnist.onnx")
    input_name = session.get_inputs()[0].name
    print(f"✓ Model loaded")
    print()
    
    times = []
    correct = 0
    
    # Warmup
    dummy = np.fromfile(test_files[0], dtype=np.float32).reshape(1, 1, 28, 28)
    for _ in range(n_warmup):
        session.run(None, {input_name: dummy})
    
    print(f"Running {len(test_files)} inferences...")
    
    # Benchmark
    for i, test_file in enumerate(test_files):
        label = int(test_file.stem.split('_')[-1])
        img = np.fromfile(test_file, dtype=np.float32).reshape(1, 1, 28, 28)
        
        start = time.perf_counter()
        output = session.run(None, {input_name: img})
        elapsed = (time.perf_counter() - start) * 1000
        
        pred = np.argmax(output[0])
        if pred == label:
            correct += 1
        
        times.append(elapsed)
        
        if (i + 1) % 20 == 0:
            print(f"  [{i+1}/{len(test_files)}] Avg: {np.mean(times):.2f} ms")
    
    avg_time = np.mean(times)
    std_time = np.std(times)
    accuracy = 100 * correct / len(test_files)
    
    print()
    print(f"✓ Inference time: {avg_time:.2f} ± {std_time:.2f} ms")
    print(f"✓ Accuracy: {accuracy:.1f}% ({correct}/{len(test_files)})")
    
    return {'mean': avg_time, 'std': std_time, 'accuracy': accuracy}


if __name__ == "__main__":
    # Get test files
    test_files = sorted(list(Path("test_data/preprocessed_28x28").glob("*.bin")))[:100]
    
    print("=" * 70)
    print("FAIR BENCHMARK: Library vs Library")
    print("=" * 70)
    print()
    print(f"Model: LeNet5")
    print(f"Test images: {len(test_files)}")
    print()
    print("Both frameworks:")
    print("  ✓ Loaded as library (DLL)")
    print("  ✓ Model loaded ONCE before timing")
    print("  ✓ Time measurement: inference function ONLY")
    print("  ✓ No process spawn overhead")
    print()
    
    try:
        netlang_results = benchmark_netlang_library(test_files)
        onnx_results = benchmark_onnxruntime(test_files)
        
        print()
        print("=" * 70)
        print("RESULTS SUMMARY (FAIR COMPARISON)")
        print("=" * 70)
        print()
        print(f"{'Framework':<20} {'Time (ms)':<20} {'Accuracy':<15} {'Speedup'}")
        print("-" * 70)
        print(f"{'ONNX Runtime':<20} {onnx_results['mean']:.2f} ± {onnx_results['std']:.2f}        "
              f"{onnx_results['accuracy']:.1f}%          baseline")
        print(f"{'NetLang (Library)':<20} {netlang_results['mean']:.2f} ± {netlang_results['std']:.2f}       "
              f"{netlang_results['accuracy']:.1f}%          "
              f"{netlang_results['mean'] / onnx_results['mean']:.1f}× slower")
        print()
        print("This is the REAL performance gap:")
        print(f"  NetLang is {netlang_results['mean'] / onnx_results['mean']:.1f}× slower than ONNX Runtime")
        print(f"  NOT 240× slower (that was unfair: subprocess vs library)")
        print()
        print("Both use same methodology:")
        print("  - Load model once")
        print("  - Time inference function only")
        print("  - No I/O or process overhead")
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        print("\nMake sure you've built the DLL:")
        print("  .\\build_library.bat generated\\lenet5.c netlang_lenet5")
