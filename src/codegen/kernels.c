/*
 * NetLang Optimized Kernels Implementation
 * 
 * AVX2-optimized Conv2D and Dense layers for maximum performance.
 */

#include "kernels.h"
#include <math.h>
#include <string.h>
#include <float.h>

/* ========== ACTIVATION FUNCTIONS ========== */

void relu_inplace_avx2(float* data, int size) {
    __m256 zero = _mm256_setzero_ps();
    int i;
    
    /* Process 8 elements at a time with AVX2 */
    for (i = 0; i <= size - 8; i += 8) {
        __m256 v = _mm256_load_ps(&data[i]);
        v = _mm256_max_ps(v, zero);  /* max(v, 0) */
        _mm256_store_ps(&data[i], v);
    }
    
    /* Handle remaining elements */
    for (; i < size; i++) {
        if (data[i] < 0.0f) data[i] = 0.0f;
    }
}

void sigmoid_inplace_avx2(float* data, int size) {
    /* sigmoid(x) = 1 / (1 + exp(-x)) */
    for (int i = 0; i < size; i++) {
        data[i] = 1.0f / (1.0f + expf(-data[i]));
    }
}

void softmax_inplace(float* data, int size) {
    /* Find max for numerical stability */
    float max_val = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] > max_val) max_val = data[i];
    }
    
    /* Compute exp(x - max) and sum */
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        data[i] = expf(data[i] - max_val);
        sum += data[i];
    }
    
    /* Normalize */
    float inv_sum = 1.0f / sum;
    for (int i = 0; i < size; i++) {
        data[i] *= inv_sum;
    }
}

/* ========== CONV2D KERNEL ========== */

void conv2d_forward_avx2(
    const float* input,
    const float* weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding
) {
    /* Calculate output dimensions */
    int H_out = (H_in + 2 * padding - K_h) / stride + 1;
    int W_out = (W_in + 2 * padding - K_w) / stride + 1;
    
    /* Zero output initially */
    memset(output, 0, H_out * W_out * C_out * sizeof(float));
    
    /* For each output channel */
    for (int oc = 0; oc < C_out; oc++) {
        float bias_val = bias ? bias[oc] : 0.0f;
        __m256 bias_vec = _mm256_set1_ps(bias_val);
        
        /* For each output row */
        for (int oh = 0; oh < H_out; oh++) {
            int h_start = oh * stride - padding;
            
            /* For each output column (process 8 at a time with AVX2) */
            for (int ow = 0; ow < W_out; ow += 8) {
                int batch_size = (ow + 8 <= W_out) ? 8 : (W_out - ow);
                
                __m256 sum[8];
                for (int b = 0; b < batch_size; b++) {
                    sum[b] = _mm256_setzero_ps();
                }
                
                /* Convolve with kernel */
                for (int kh = 0; kh < K_h; kh++) {
                    int h = h_start + kh;
                    if (h < 0 || h >= H_in) continue;  /* Padding check */
                    
                    for (int kw = 0; kw < K_w; kw++) {
                        for (int ic = 0; ic < C_in; ic++) {
                            /* Weight for this position */
                            int w_idx = ((kh * K_w + kw) * C_in + ic) * C_out + oc;
                            __m256 weight_vec = _mm256_set1_ps(weights[w_idx]);
                            
                            /* Gather inputs for 8 output pixels */
                            for (int b = 0; b < batch_size; b++) {
                                int w = (ow + b) * stride - padding + kw;
                                
                                if (w >= 0 && w < W_in) {
                                    int in_idx = (h * W_in + w) * C_in + ic;
                                    __m256 inp_vec = _mm256_set1_ps(input[in_idx]);
                                    
                                    /* FMA: sum = sum + input * weight */
                                    sum[b] = _mm256_fmadd_ps(inp_vec, weight_vec, sum[b]);
                                }
                            }
                        }
                    }
                }
                
                /* Store results (horizontal sum + bias) */
                for (int b = 0; b < batch_size; b++) {
                    /* Horizontal sum of 8 elements */
                    __m256 hadd = _mm256_hadd_ps(sum[b], sum[b]);
                    hadd = _mm256_hadd_ps(hadd, hadd);
                    float result = ((float*)&hadd)[0] + ((float*)&hadd)[4];
                    
                    int out_idx = (oh * W_out + ow + b) * C_out + oc;
                    output[out_idx] = result + bias_val;
                }
            }
        }
    }
}

/* ========== DENSE KERNEL ========== */

void dense_forward_avx2(
    const float* input,
    const float* weights,
    const float* bias,
    float* output,
    int in_features,
    int out_features
) {
    /* For each output neuron */
    for (int out = 0; out < out_features; out++) {
        __m256 sum = _mm256_setzero_ps();
        int in;
        
        /* Vectorized dot product (process 8 inputs at a time) */
        for (in = 0; in <= in_features - 8; in += 8) {
            /* Load 8 input values */
            __m256 inp = _mm256_loadu_ps(&input[in]);
            
            /* Load 8 weights for this output */
            __m256 wgt = _mm256_loadu_ps(&weights[in * out_features + out]);
            
            /* FMA: sum += input * weight */
            sum = _mm256_fmadd_ps(inp, wgt, sum);
        }
        
        /* Horizontal sum of 8 partial sums */
        __m256 hadd = _mm256_hadd_ps(sum, sum);
        hadd = _mm256_hadd_ps(hadd, hadd);
        float result = ((float*)&hadd)[0] + ((float*)&hadd)[4];
        
        /* Handle remaining elements (scalar) */
        for (; in < in_features; in++) {
            result += input[in] * weights[in * out_features + out];
        }
        
        /* Add bias */
        output[out] = result + (bias ? bias[out] : 0.0f);
    }
}

/* ========== POOLING KERNELS ========== */

void maxpool2d_forward(
    const float* input,
    float* output,
    int H_in, int W_in, int C,
    int pool_h, int pool_w,
    int stride,
    int padding
) {
    int H_out = (H_in + 2 * padding - pool_h) / stride + 1;
    int W_out = (W_in + 2 * padding - pool_w) / stride + 1;
    
    for (int c = 0; c < C; c++) {
        for (int oh = 0; oh < H_out; oh++) {
            for (int ow = 0; ow < W_out; ow++) {
                float max_val = -FLT_MAX;
                
                /* Find max in pooling window */
                for (int ph = 0; ph < pool_h; ph++) {
                    int h = oh * stride - padding + ph;
                    if (h < 0 || h >= H_in) continue;
                    
                    for (int pw = 0; pw < pool_w; pw++) {
                        int w = ow * stride - padding + pw;
                        if (w < 0 || w >= W_in) continue;
                        
                        int in_idx = (h * W_in + w) * C + c;
                        if (input[in_idx] > max_val) {
                            max_val = input[in_idx];
                        }
                    }
                }
                
                int out_idx = (oh * W_out + ow) * C + c;
                output[out_idx] = max_val;
            }
        }
    }
}

void avgpool2d_forward(
    const float* input,
    float* output,
    int H_in, int W_in, int C,
    int pool_h, int pool_w,
    int stride,
    int padding
) {
    int H_out = (H_in + 2 * padding - pool_h) / stride + 1;
    int W_out = (W_in + 2 * padding - pool_w) / stride + 1;
    
    for (int c = 0; c < C; c++) {
        for (int oh = 0; oh < H_out; oh++) {
            for (int ow = 0; ow < W_out; ow++) {
                float sum = 0.0f;
                int count = 0;
                
                /* Sum values in pooling window */
                for (int ph = 0; ph < pool_h; ph++) {
                    int h = oh * stride - padding + ph;
                    if (h < 0 || h >= H_in) continue;
                    
                    for (int pw = 0; pw < pool_w; pw++) {
                        int w = ow * stride - padding + pw;
                        if (w < 0 || w >= W_in) continue;
                        
                        int in_idx = (h * W_in + w) * C + c;
                        sum += input[in_idx];
                        count++;
                    }
                }
                
                int out_idx = (oh * W_out + ow) * C + c;
                output[out_idx] = sum / count;
            }
        }
    }
}

/* ========== UTILITY KERNELS ========== */

void flatten(const float* input, float* output, int size) {
    /* Simple memcpy - data already in row-major format */
    memcpy(output, input, size * sizeof(float));
}

void concat_forward(
    const float** inputs,
    float* output,
    int num_inputs,
    int H, int W,
    const int* input_channels,
    int total_channels
) {
    /* Concatenate along channel dimension */
    int out_offset = 0;
    
    for (int input_idx = 0; input_idx < num_inputs; input_idx++) {
        int C = input_channels[input_idx];
        
        for (int h = 0; h < H; h++) {
            for (int w = 0; w < W; w++) {
                /* Copy C channels from this input */
                const float* src = &inputs[input_idx][(h * W + w) * C];
                float* dst = &output[(h * W + w) * total_channels + out_offset];
                memcpy(dst, src, C * sizeof(float));
            }
        }
        
        out_offset += C;
    }
}
