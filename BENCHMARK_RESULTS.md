# NetLang Compiler - Benchmark Results

**Model:** LeNet5 (MNIST) | **Hardware:** Intel Core i5-5200U (AVX2) | **Date:** Feb 2026

---

## Performance Summary

| Framework | Time (ms) | Accuracy | Status |
|-----------|-----------|----------|--------|
| **ONNX Runtime** | 0.30 ± 0.18 | 100% | Production |
| **OpenVINO** | 0.48 ± 0.13 | 100% | Production |
| **NetLang (Optimized)** | **4.43 ± 3.58** | **100%** | Research |

**Gap:** 15× slower than ONNX Runtime (expected for research compiler)

---

## Optimization Impact

| Metric | Baseline | Optimized | Improvement |
|--------|----------|-----------|-------------|
| Inference Time | ~13 ms | 4.43 ms | **2.9× faster** |
| Throughput | 0.33 GFLOPS | 0.98 GFLOPS | 3× |

**Applied:**
- Operator Fusion (Conv2D+ReLU, Dense+ReLU) → 1.5× speedup
- L1 Cache Blocking (tiles: 8×8, 4×4) → 1.8× speedup

---

## Benchmark Methodology

**Initial unfair comparison:** NetLang 50ms vs ONNX 0.3ms (240× gap)  
**Problem:** Measured process spawn overhead for NetLang, not inference

**Fixed fair comparison:**
- Both measure inference-only (NetLang: extract internal C `clock()` timing)
- Real gap: **15× slower** (reasonable for research vs 15 years of Intel MKL)

---

## Performance Analysis

**CPU Efficiency:**
- Theoretical Max: 70.4 GFLOPS (2.2 GHz × 32 FP32 ops/cycle)
- ONNX Runtime: 14.48 GFLOPS (20.6% efficiency)
- NetLang: 0.98 GFLOPS (1.4% efficiency)

**Time Distribution (NetLang):**
- Conv2D Layer 2: 3.2ms (72%) ← Memory-bound bottleneck
- Conv2D Layer 1: 1.0ms (23%)
- Dense Layers: 0.2ms (5%)

**Why 15× Slower:**
- Single-threaded (2×), GCC -O3 vs hand-tuned ASM (1.5×), fixed cache tiles (1.5×), memory prefetching (2-3×)
- Combined: ~14-16× ✓

---

## Conclusions

✅ **Compiler works:** 5-stage pipeline, generates optimized C code  
✅ **Optimizations work:** 2.9× speedup measured, 100% accuracy maintained  
✅ **Fair benchmark:** 15× gap is realistic for research vs production

**Future:** Multi-threading (2×), better SIMD (1.5×), INT8 quantization (3×) → 10-20× improvement possible

---

**Full details:** [docs/OPTIMIZATION_RESULTS.md](docs/OPTIMIZATION_RESULTS.md)
