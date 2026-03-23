/*
 * NetLang Optimized Kernels - AVX2 SIMD Implementation
 * 
 * High-performance Conv2D and Dense layer kernels optimized for
 * Intel Core i5-5200U (Haswell) with AVX2 and FMA support.
 * 
 * Optimizations:
 * - AVX2 vectorization (8x float32 per instruction)
 * - FMA (Fused Multiply-Add) for 2x throughput
 * - Cache blocking/tiling for L1/L2 efficiency
 * - Loop unrolling
 * - Data prefetching
 * 
 * Author: Abhijeet Deb Nath
 * Target: x86-64 AVX2
 */

#ifndef NETLANG_KERNELS_H
#define NETLANG_KERNELS_H

#include "runtime.h"
#include <immintrin.h>  /* AVX2 intrinsics */

/* ========== ACTIVATION FUNCTIONS ========== */

void relu_inplace_avx2(float* data, int size);
void sigmoid_inplace_avx2(float* data, int size);
void softmax_inplace(float* data, int size);

/* ========== CONV2D KERNELS ========== */

/**
 * AVX2-optimized Conv2D forward pass
 * 
 * Input shape:  [H_in, W_in, C_in]
 * Weight shape: [K_h, K_w, C_in, C_out]
 * Bias shape:   [C_out]
 * Output shape: [H_out, W_out, C_out]
 * 
 * Formula:
 *   H_out = (H_in + 2*padding - K_h) / stride + 1
 *   W_out = (W_in + 2*padding - K_w) / stride + 1
 */
void conv2d_forward_avx2(
    const float* input,     /* Input tensor [H_in, W_in, C_in] */
    const float* weights,   /* Kernel weights [K_h, K_w, C_in, C_out] */
    const float* bias,      /* Bias [C_out], can be NULL */
    float* output,          /* Output tensor [H_out, W_out, C_out] */
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding
);

/**
 * Conv2D + ReLU fused (15-30% faster than separate operations)
 */
void conv2d_relu_forward_avx2(
    const float* input,
    const float* weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding
);

/**
 * Conv2D with L1 cache blocking (3-5× faster for medium/large layers)
 * Uses tiling to keep working set in L1 cache (32 KB)
 */
void conv2d_blocked_avx2(
    const float* input,
    const float* weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding,
    int tile_size       /* Tile dimension (e.g., 8 for 8×8 tiles) */
);

/**
 * Conv2D + ReLU fused with L1 cache blocking (combined optimization)
 * Best performance: fuses ReLU and uses cache-efficient tiling
 */
void conv2d_relu_blocked_avx2(
    const float* input,
    const float* weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding,
    int tile_size
);

/* ========== DENSE (FULLY CONNECTED) KERNELS ========== */

/**
 * AVX2-optimized Dense/Linear layer
 * 
 * Matrix multiplication: output = input * weights + bias
 * 
 * Input shape:  [in_features]
 * Weight shape: [in_features, out_features]
 * Bias shape:   [out_features]
 * Output shape: [out_features]
 */
void dense_forward_avx2(
    const float* input,     /* Input vector [in_features] */
    const float* weights,   /* Weight matrix [in_features, out_features] */
    const float* bias,      /* Bias vector [out_features], can be NULL */
    float* output,          /* Output vector [out_features] */
    int in_features,
    int out_features
);

/**
 * Dense + ReLU fused (15-30% faster than separate operations)
 */
void dense_relu_forward_avx2(
    const float* input,
    const float* weights,
    const float* bias,
    float* output,
    int in_features,
    int out_features
);

/* ========== POOLING KERNELS ========== */

/**
 * MaxPooling2D
 * 
 * Input shape:  [H_in, W_in, C]
 * Output shape: [H_out, W_out, C]
 * 
 * H_out = (H_in - pool_h) / stride + 1
 * W_out = (W_in - pool_w) / stride + 1
 */
void maxpool2d_forward(
    const float* input,
    float* output,
    int H_in, int W_in, int C,
    int pool_h, int pool_w,
    int stride,
    int padding
);

/**
 * AvgPooling2D
 */
void avgpool2d_forward(
    const float* input,
    float* output,
    int H_in, int W_in, int C,
    int pool_h, int pool_w,
    int stride,
    int padding
);

/* ========== UTILITY KERNELS ========== */

/**
 * Flatten 3D tensor to 1D
 */
void flatten(const float* input, float* output, int size);

/**
 * Flatten HWC to CHW order for compatibility with PyTorch/ONNX dense layers
 * Input is [H][W][C], output is [C][H][W] flattened
 */
void flatten_hwc_to_chw(const float* input, float* output, int H, int W, int C);

/**
 * Elementwise sum of multiple tensors with identical shapes.
 */
void add_forward(
    const float** inputs,
    float* output,
    int num_inputs,
    int size
);

/**
 * Concatenate multiple tensors along channel dimension
 */
void concat_forward(
    const float** inputs,   /* Array of input tensors */
    float* output,          /* Output tensor */
    int num_inputs,
    int H, int W,
    const int* input_channels,  /* Channels for each input */
    int total_channels      /* Sum of all input channels */
);

#endif /* NETLANG_KERNELS_H */
