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

static int round_up_to_8(int value) {
    return (value + 7) & ~7;
}

typedef struct PackedConvTask {
    const float* input;
    const float* packed_weights;
    const float* bias;
    float* output;
    int H_in;
    int W_in;
    int C_in;
    int K_h;
    int K_w;
    int C_out;
    int stride;
    int padding;
    int tile_h_step;
    int tile_w_step;
    int H_out;
    int W_out;
    int padded_out;
    int apply_relu;
    int spatial_block_width;
} PackedConvTask;

static void packed_conv_compute_output_block(const PackedConvTask* task,
                                             int oc,
                                             __m256 bias_vec,
                                             __m256 zero,
                                             int oh,
                                             int ow,
                                             int output_count) {
    __m256 sums[4];
    float out_block[8];

    for (int lane = 0; lane < output_count; lane++) {
        sums[lane] = bias_vec;
    }

    for (int kh = 0; kh < task->K_h; kh++) {
        int h = oh * task->stride - task->padding + kh;
        if (h < 0 || h >= task->H_in) {
            continue;
        }

        for (int kw = 0; kw < task->K_w; kw++) {
            const float* input_pixels[4] = {0};
            const float* packed_base = NULL;

            for (int lane = 0; lane < output_count; lane++) {
                int w = (ow + lane) * task->stride - task->padding + kw;
                if (w < 0 || w >= task->W_in) {
                    continue;
                }
                input_pixels[lane] = &task->input[(h * task->W_in + w) * task->C_in];
            }

            packed_base = task->packed_weights + ((((kh * task->K_w) + kw) * task->C_in) * task->padded_out + oc);

            for (int ic = 0; ic < task->C_in; ic++) {
                __m256 weight_vec = _mm256_load_ps(packed_base + ic * task->padded_out);

                for (int lane = 0; lane < output_count; lane++) {
                    if (!input_pixels[lane]) {
                        continue;
                    }
                    __m256 input_vec = _mm256_set1_ps(input_pixels[lane][ic]);
                    sums[lane] = _mm256_fmadd_ps(input_vec, weight_vec, sums[lane]);
                }
            }
        }
    }

    if (task->apply_relu) {
        for (int lane = 0; lane < output_count; lane++) {
            sums[lane] = _mm256_max_ps(sums[lane], zero);
        }
    }

    for (int lane = 0; lane < output_count; lane++) {
        int out_base = (oh * task->W_out + ow + lane) * task->C_out + oc;
        int valid_channels = (oc + 8 <= task->C_out) ? 8 : (task->C_out - oc);
        _mm256_storeu_ps(out_block, sums[lane]);

        for (int channel = 0; channel < valid_channels; channel++) {
            task->output[out_base + channel] = out_block[channel];
        }
    }
}

static void packed_conv_oc_block_range(void* user_data, int start, int end) {
    PackedConvTask* task = (PackedConvTask*)user_data;
    __m256 zero = _mm256_setzero_ps();
    int spatial_block_width = task->spatial_block_width > 0 ? task->spatial_block_width : 1;

    if (spatial_block_width > 4) {
        spatial_block_width = 4;
    }

    for (int oc_block = start; oc_block < end; oc_block++) {
        int oc = oc_block * 8;
        float bias_block[8];

        for (int lane = 0; lane < 8; lane++) {
            int out_channel = oc + lane;
            bias_block[lane] = (task->bias && out_channel < task->C_out) ? task->bias[out_channel] : 0.0f;
        }

        __m256 bias_vec = _mm256_loadu_ps(bias_block);

        for (int tile_h = 0; tile_h < task->H_out; tile_h += task->tile_h_step) {
            int tile_h_end = (tile_h + task->tile_h_step < task->H_out) ? (tile_h + task->tile_h_step) : task->H_out;

            for (int tile_w = 0; tile_w < task->W_out; tile_w += task->tile_w_step) {
                int tile_w_end = (tile_w + task->tile_w_step < task->W_out) ? (tile_w + task->tile_w_step) : task->W_out;

                for (int oh = tile_h; oh < tile_h_end; oh++) {
                    for (int ow = tile_w; ow < tile_w_end; ow += spatial_block_width) {
                        int output_count = spatial_block_width;
                        if (ow + output_count > tile_w_end) {
                            output_count = tile_w_end - ow;
                        }
                        packed_conv_compute_output_block(task, oc, bias_vec, zero, oh, ow, output_count);
                    }
                }
            }
        }
    }
}

float* repack_conv2d_weights_oc8(
    const float* weights,
    int C_out,
    int C_in,
    int K_h,
    int K_w
) {
    int padded_out = round_up_to_8(C_out);
    size_t total_elements = (size_t)K_h * (size_t)K_w * (size_t)C_in * (size_t)padded_out;
    float* packed = NULL;

    if (!weights || C_out <= 0 || C_in <= 0 || K_h <= 0 || K_w <= 0) {
        return NULL;
    }

    packed = (float*)aligned_alloc_64(total_elements * sizeof(float));
    if (!packed) {
        return NULL;
    }

    memset(packed, 0, total_elements * sizeof(float));

    for (int kh = 0; kh < K_h; kh++) {
        for (int kw = 0; kw < K_w; kw++) {
            for (int ic = 0; ic < C_in; ic++) {
                float* dst = packed + ((((kh * K_w) + kw) * C_in + ic) * padded_out);
                for (int oc = 0; oc < C_out; oc++) {
                    int src_idx = ((oc * C_in + ic) * K_h + kh) * K_w + kw;
                    dst[oc] = weights[src_idx];
                }
            }
        }
    }

    return packed;
}

static void conv2d_packed_oc8_core(
    const float* input,
    const float* packed_weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding,
    int spatial_block_width,
    int tile_size,
    int apply_relu,
    NetLangThreadPool* thread_pool
) {
    int H_out = (H_in + 2 * padding - K_h) / stride + 1;
    int W_out = (W_in + 2 * padding - K_w) / stride + 1;
    int padded_out = round_up_to_8(C_out);
    int tile_h_step = tile_size > 0 ? tile_size : H_out;
    int tile_w_step = tile_size > 0 ? tile_size : W_out;
    int oc_block_count = padded_out / 8;
    long long total_ops = (long long)H_out * (long long)W_out * (long long)K_h * (long long)K_w *
                          (long long)C_in * (long long)padded_out;
    int use_parallel = thread_pool &&
                       netlang_thread_pool_thread_count(thread_pool) > 1 &&
                       oc_block_count > 1 &&
                       total_ops >= 200000;
    PackedConvTask task;

    task.input = input;
    task.packed_weights = packed_weights;
    task.bias = bias;
    task.output = output;
    task.H_in = H_in;
    task.W_in = W_in;
    task.C_in = C_in;
    task.K_h = K_h;
    task.K_w = K_w;
    task.C_out = C_out;
    task.stride = stride;
    task.padding = padding;
    task.tile_h_step = tile_h_step;
    task.tile_w_step = tile_w_step;
    task.H_out = H_out;
    task.W_out = W_out;
    task.padded_out = padded_out;
    task.apply_relu = apply_relu;
    task.spatial_block_width = spatial_block_width;

    if (use_parallel) {
        netlang_parallel_for(thread_pool, 0, oc_block_count, 1, packed_conv_oc_block_range, &task);
    } else {
        packed_conv_oc_block_range(&task, 0, oc_block_count);
    }
}

void conv2d_packed_oc8_forward_avx2(
    const float* input,
    const float* packed_weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding,
    int spatial_block_width,
    NetLangThreadPool* thread_pool
) {
    conv2d_packed_oc8_core(
        input, packed_weights, bias, output,
        H_in, W_in, C_in, K_h, K_w, C_out, stride, padding,
        spatial_block_width, 0, 0, thread_pool
    );
}

void conv2d_relu_packed_oc8_forward_avx2(
    const float* input,
    const float* packed_weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding,
    int spatial_block_width,
    NetLangThreadPool* thread_pool
) {
    conv2d_packed_oc8_core(
        input, packed_weights, bias, output,
        H_in, W_in, C_in, K_h, K_w, C_out, stride, padding,
        spatial_block_width, 0, 1, thread_pool
    );
}

void conv2d_packed_oc8_blocked_avx2(
    const float* input,
    const float* packed_weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding,
    int tile_size,
    int spatial_block_width,
    NetLangThreadPool* thread_pool
) {
    conv2d_packed_oc8_core(
        input, packed_weights, bias, output,
        H_in, W_in, C_in, K_h, K_w, C_out, stride, padding,
        spatial_block_width, tile_size, 0, thread_pool
    );
}

void conv2d_relu_packed_oc8_blocked_avx2(
    const float* input,
    const float* packed_weights,
    const float* bias,
    float* output,
    int H_in, int W_in, int C_in,
    int K_h, int K_w,
    int C_out,
    int stride,
    int padding,
    int tile_size,
    int spatial_block_width,
    NetLangThreadPool* thread_pool
) {
    conv2d_packed_oc8_core(
        input, packed_weights, bias, output,
        H_in, W_in, C_in, K_h, K_w, C_out, stride, padding,
        spatial_block_width, tile_size, 1, thread_pool
    );
}

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
                            /* Weight layout: OIHW [out_ch][in_ch][kh][kw] */
                            int w_idx = ((oc * C_in + ic) * K_h + kh) * K_w + kw;
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
    /* NOTE: .nwf format stores Dense weights as [out_features, in_features] (transposed)
     * This enables sequential access for each output neuron and aligned loads.
     */
    
    /* For each output neuron */
    for (int out = 0; out < out_features; out++) {
        __m256 sum = _mm256_setzero_ps();
        int in;
        
        /* Base offset for this output's weights (sequential in .nwf format) */
        const float* out_weights = &weights[out * in_features];
        
        /* Vectorized dot product (process 8 inputs at a time) */
        for (in = 0; in <= in_features - 8; in += 8) {
            /* Load 8 input values - ALIGNED (from aligned_alloc_64) */
            __m256 inp = _mm256_load_ps(&input[in]);
            
            /* Load 8 weights for this output - ALIGNED (from .nwf format) */
            __m256 wgt = _mm256_load_ps(&out_weights[in]);
            
            /* FMA: sum += input * weight */
            sum = _mm256_fmadd_ps(inp, wgt, sum);
        }
        
        /* Horizontal sum of 8 partial sums */
        __m256 hadd = _mm256_hadd_ps(sum, sum);
        hadd = _mm256_hadd_ps(hadd, hadd);
        float result = ((float*)&hadd)[0] + ((float*)&hadd)[4];
        
        /* Handle remaining elements (scalar) */
        for (; in < in_features; in++) {
            result += input[in] * out_weights[in];
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

void flatten_hwc_to_chw(const float* input, float* output, int H, int W, int C) {
    /*
     * Convert HWC [H][W][C] layout to CHW [C][H][W] layout
     * This is needed for compatibility with PyTorch/ONNX dense layers
     * which expect CHW flattening
     *
     * Input index:  (h * W + w) * C + c
     * Output index: c * H * W + h * W + w
     */
    for (int c = 0; c < C; c++) {
        for (int h = 0; h < H; h++) {
            for (int w = 0; w < W; w++) {
                int in_idx = (h * W + w) * C + c;
                int out_idx = c * H * W + h * W + w;
                output[out_idx] = input[in_idx];
            }
        }
    }
}

float* repack_dense_weights_chw_to_hwc(
    const float* weights,
    int out_features,
    int H, int W, int C
) {
    int in_features = H * W * C;
    float* repacked = NULL;

    if (!weights || out_features <= 0 || H <= 0 || W <= 0 || C <= 0) {
        return NULL;
    }

    repacked = (float*)aligned_alloc_64((size_t)out_features * (size_t)in_features * sizeof(float));
    if (!repacked) {
        return NULL;
    }

    for (int out = 0; out < out_features; out++) {
        const float* src_row = &weights[out * in_features];
        float* dst_row = &repacked[out * in_features];

        for (int h = 0; h < H; h++) {
            for (int w = 0; w < W; w++) {
                for (int c = 0; c < C; c++) {
                    int chw_index = c * H * W + h * W + w;
                    int hwc_index = (h * W + w) * C + c;
                    dst_row[hwc_index] = src_row[chw_index];
                }
            }
        }
    }

    return repacked;
}

void add_forward(
    const float** inputs,
    float* output,
    int num_inputs,
    int size
) {
    for (int idx = 0; idx < size; idx++) {
        float sum = 0.0f;
        for (int input_idx = 0; input_idx < num_inputs; input_idx++) {
            sum += inputs[input_idx][idx];
        }
        output[idx] = sum;
    }
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

void extract_hwc_tile_zero_pad(
    const float* input,
    float* output,
    int H_in, int W_in, int C,
    int src_h0, int src_w0,
    int tile_h, int tile_w
) {
    memset(output, 0, (size_t)tile_h * (size_t)tile_w * (size_t)C * sizeof(float));

    for (int th = 0; th < tile_h; th++) {
        int src_h = src_h0 + th;
        if (src_h < 0 || src_h >= H_in) {
            continue;
        }

        for (int tw = 0; tw < tile_w; tw++) {
            int src_w = src_w0 + tw;
            if (src_w < 0 || src_w >= W_in) {
                continue;
            }

            const float* src = &input[(src_h * W_in + src_w) * C];
            float* dst = &output[(th * tile_w + tw) * C];
            memcpy(dst, src, (size_t)C * sizeof(float));
        }
    }
}

void scatter_hwc_tile(
    const float* tile,
    float* output,
    int H_out, int W_out, int C,
    int dst_h0, int dst_w0,
    int tile_h, int tile_w
) {
    for (int th = 0; th < tile_h; th++) {
        int dst_h = dst_h0 + th;
        if (dst_h < 0 || dst_h >= H_out) {
            continue;
        }

        for (int tw = 0; tw < tile_w; tw++) {
            int dst_w = dst_w0 + tw;
            if (dst_w < 0 || dst_w >= W_out) {
                continue;
            }

            const float* src = &tile[(th * tile_w + tw) * C];
            float* dst = &output[(dst_h * W_out + dst_w) * C];
            memcpy(dst, src, (size_t)C * sizeof(float));
        }
    }
}

/* ========== FUSED OPERATOR KERNELS ========== */

/**
 * Conv2D + ReLU fused kernel
 * Applies ReLU activation inline during convolution, saving memory bandwidth
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
) {
    int H_out = (H_in + 2 * padding - K_h) / stride + 1;
    int W_out = (W_in + 2 * padding - K_w) / stride + 1;
    
    /* Zero output initially */
    memset(output, 0, H_out * W_out * C_out * sizeof(float));
    
    __m256 zero_vec = _mm256_setzero_ps();
    
    /* For each output channel */
    for (int oc = 0; oc < C_out; oc++) {
        float bias_val = bias ? bias[oc] : 0.0f;
        
        /* For each output row */
        for (int oh = 0; oh < H_out; oh++) {
            int h_start = oh * stride - padding;
            
            /* For each output column (process 8 at a time) */
            for (int ow = 0; ow < W_out; ow += 8) {
                int batch_size = (ow + 8 <= W_out) ? 8 : (W_out - ow);
                
                __m256 sum[8];
                for (int b = 0; b < batch_size; b++) {
                    sum[b] = _mm256_setzero_ps();
                }
                
                /* Convolve with kernel */
                for (int kh = 0; kh < K_h; kh++) {
                    int h = h_start + kh;
                    if (h < 0 || h >= H_in) continue;
                    
                    for (int kw = 0; kw < K_w; kw++) {
                        for (int ic = 0; ic < C_in; ic++) {
                            /* Weight layout: OIHW [out_ch][in_ch][kh][kw] */
                            int w_idx = ((oc * C_in + ic) * K_h + kh) * K_w + kw;
                            __m256 weight_vec = _mm256_set1_ps(weights[w_idx]);
                            
                            for (int b = 0; b < batch_size; b++) {
                                int w = (ow + b) * stride - padding + kw;
                                
                                if (w >= 0 && w < W_in) {
                                    int in_idx = (h * W_in + w) * C_in + ic;
                                    __m256 inp_vec = _mm256_set1_ps(input[in_idx]);
                                    sum[b] = _mm256_fmadd_ps(inp_vec, weight_vec, sum[b]);
                                }
                            }
                        }
                    }
                }
                
                /* Store results with FUSED ReLU (max(0, result)) */
                for (int b = 0; b < batch_size; b++) {
                    __m256 hadd = _mm256_hadd_ps(sum[b], sum[b]);
                    hadd = _mm256_hadd_ps(hadd, hadd);
                    float result = ((float*)&hadd)[0] + ((float*)&hadd)[4] + bias_val;
                    
                    /* Apply ReLU inline: max(0, result) */
                    result = (result > 0.0f) ? result : 0.0f;
                    
                    int out_idx = (oh * W_out + ow + b) * C_out + oc;
                    output[out_idx] = result;
                }
            }
        }
    }
}

/**
 * Dense + ReLU fused kernel
 * Applies ReLU activation inline during matrix multiplication
 */
void dense_relu_forward_avx2(
    const float* input,
    const float* weights,
    const float* bias,
    float* output,
    int in_features,
    int out_features
) {
    __m256 zero_vec = _mm256_setzero_ps();
    
    /* For each output neuron */
    for (int out = 0; out < out_features; out++) {
        __m256 sum = _mm256_setzero_ps();
        int in;
        
        /* Base offset for this output's weights */
        const float* out_weights = &weights[out * in_features];
        
        /* Vectorized dot product */
        for (in = 0; in <= in_features - 8; in += 8) {
            __m256 inp = _mm256_load_ps(&input[in]);
            __m256 wgt = _mm256_load_ps(&out_weights[in]);
            sum = _mm256_fmadd_ps(inp, wgt, sum);
        }
        
        /* Horizontal sum */
        __m256 hadd = _mm256_hadd_ps(sum, sum);
        hadd = _mm256_hadd_ps(hadd, hadd);
        float result = ((float*)&hadd)[0] + ((float*)&hadd)[4];
        
        /* Handle remaining elements */
        for (; in < in_features; in++) {
            result += input[in] * out_weights[in];
        }
        
        /* Add bias and apply FUSED ReLU */
        result += (bias ? bias[out] : 0.0f);
        output[out] = (result > 0.0f) ? result : 0.0f;
    }
}

/* ========== CACHE-BLOCKED CONV2D KERNELS ========== */

/**
 * Conv2D with L1 cache blocking  
 * 
 * Processes output in tiles to keep working set in L1 cache (32 KB).
 * For a tile_size×tile_size tile, the working set is:
 *   - Input region: (tile_size + K-1)² × C_in × 4 bytes
 *   - Output region: tile_size² × C_out × 4 bytes
 *   - Total should fit in ~24 KB for optimal L1 utilization
 * 
 * Typical speedup: 3-5× for medium layers (24×24 to 112×112)
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
    int tile_size
) {
    int H_out = (H_in + 2 * padding - K_h) / stride + 1;
    int W_out = (W_in + 2 * padding - K_w) / stride + 1;
    
    memset(output, 0, H_out * W_out * C_out * sizeof(float));
    
    /* Tile over output spatial dimensions */
    for (int tile_h = 0; tile_h < H_out; tile_h += tile_size) {
        int tile_h_end = (tile_h + tile_size < H_out) ? (tile_h + tile_size) : H_out;
        
        for (int tile_w = 0; tile_w < W_out; tile_w += tile_size) {
            int tile_w_end = (tile_w + tile_size < W_out) ? (tile_w + tile_size) : W_out;
            
            /* Process tile - all output channels for this spatial region */
            for (int oc = 0; oc < C_out; oc++) {
                float bias_val = bias ? bias[oc] : 0.0f;
                
                /* Process this tile's output pixels */
                for (int oh = tile_h; oh < tile_h_end; oh++) {
                    int h_start = oh * stride - padding;
                    
                    for (int ow = tile_w; ow < tile_w_end; ow++) {
                        int w_start = ow * stride - padding;
                        float sum = 0.0f;
                        
                        /* Convolve with kernel */
                        for (int kh = 0; kh < K_h; kh++) {
                            int h = h_start + kh;
                            if (h < 0 || h >= H_in) continue;
                            
                            for (int kw = 0; kw < K_w; kw++) {
                                int w = w_start + kw;
                                if (w < 0 || w >= W_in) continue;
                                
                                for (int ic = 0; ic < C_in; ic++) {
                                    int w_idx = ((oc * C_in + ic) * K_h + kh) * K_w + kw;
                                    int in_idx = (h * W_in + w) * C_in + ic;
                                    sum += input[in_idx] * weights[w_idx];
                                }
                            }
                        }
                        
                        int out_idx = (oh * W_out + ow) * C_out + oc;
                        output[out_idx] = sum + bias_val;
                    }
                }
            }
        }
    }
}

/**
 * Conv2D + ReLU fused with L1 cache blocking
 * 
 * Combines operator fusion and cache blocking for maximum performance.
 * This is the BEST performing kernel for most layers.
 * 
 * Expected speedup: 8-13× over naive (2.5× fusion + 3-5× blocking)
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
) {
    int H_out = (H_in + 2 * padding - K_h) / stride + 1;
    int W_out = (W_in + 2 * padding - K_w) / stride + 1;
    
    memset(output, 0, H_out * W_out * C_out * sizeof(float));
    
    /* Tile over output spatial dimensions */
    for (int tile_h = 0; tile_h < H_out; tile_h += tile_size) {
        int tile_h_end = (tile_h + tile_size < H_out) ? (tile_h + tile_size) : H_out;
        
        for (int tile_w = 0; tile_w < W_out; tile_w += tile_size) {
            int tile_w_end = (tile_w + tile_size < W_out) ? (tile_w + tile_size) : W_out;
            
            /* Process tile - all output channels for this spatial region */
            for (int oc = 0; oc < C_out; oc++) {
                float bias_val = bias ? bias[oc] : 0.0f;
                
                /* Process this tile's output pixels */
                for (int oh = tile_h; oh < tile_h_end; oh++) {
                    int h_start = oh * stride - padding;
                    
                    for (int ow = tile_w; ow < tile_w_end; ow++) {
                        int w_start = ow * stride - padding;
                        float sum = 0.0f;
                        
                        /* Convolve with kernel */
                        for (int kh = 0; kh < K_h; kh++) {
                            int h = h_start + kh;
                            if (h < 0 || h >= H_in) continue;
                            
                            for (int kw = 0; kw < K_w; kw++) {
                                int w = w_start + kw;
                                if (w < 0 || w >= W_in) continue;
                                
                                for (int ic = 0; ic < C_in; ic++) {
                                    int w_idx = ((oc * C_in + ic) * K_h + kh) * K_w + kw;
                                    int in_idx = (h * W_in + w) * C_in + ic;
                                    sum += input[in_idx] * weights[w_idx];
                                }
                            }
                        }
                        
                        /* Apply bias and FUSED ReLU */
                        int out_idx = (oh * W_out + ow) * C_out + oc;
                        float result = sum + bias_val;
                        output[out_idx] = (result > 0.0f) ? result : 0.0f;
                    }
                }
            }
        }
    }
}
