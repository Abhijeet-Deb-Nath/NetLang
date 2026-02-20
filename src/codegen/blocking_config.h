/*
 * NetLang Cache Blocking Configuration
 * 
 * Adaptive tile sizing for L1/L2/L3 cache optimization.
 * Computes optimal tile sizes based on layer dimensions and target cache level.
 * 
 * Cache Hierarchy (Intel Core i5-5200U):
 * - L1: 32 KB data cache, 4 cycles latency
 * - L2: 256 KB, 12 cycles latency
 * - L3: 3 MB, 36 cycles latency
 * - RAM: 200+ cycles latency
 * 
 * Author: Abhijeet Deb Nath
 * Date: February 2026
 */

#ifndef NETLANG_BLOCKING_CONFIG_H
#define NETLANG_BLOCKING_CONFIG_H

#include <math.h>
#include <stdbool.h>

/* ========== CACHE SIZES ========== */
#define L1_CACHE_SIZE 32768     /* 32 KB */
#define L2_CACHE_SIZE 262144    /* 256 KB */
#define L3_CACHE_SIZE 3145728   /* 3 MB */

/* Working set budget (leave room for other data) */
#define L1_WORKING_SET 24576    /* 24 KB (75% of L1) */
#define L2_WORKING_SET 196608   /* 192 KB (75% of L2) */

/* ========== BLOCKING STRATEGY ========== */
typedef enum {
    BLOCK_NONE,         /* No blocking (output too small or overhead too high) */
    BLOCK_L1,           /* L1 cache blocking only */
    BLOCK_L1_L2,        /* Hierarchical L1 + L2 blocking */
    BLOCK_L1_L2_L3,     /* Full 3-level blocking (for very large layers) */
} BlockingStrategy;

/* ========== BLOCKING METADATA ========== */
typedef struct BlockingInfo {
    BlockingStrategy strategy;
    int l1_tile_size;       /* L1 tile dimension (e.g., 8 for 8×8 tiles) */
    int l2_tile_size;       /* L2 tile dimension (e.g., 32 for 32×32 tiles) */
    int l3_tile_size;       /* L3 tile dimension (unused for now) */
    bool is_depthwise;      /* Depthwise separable conv (different blocking) */
    bool is_pointwise;      /* 1×1 convolution (spatial tiling only) */
} BlockingInfo;

/* ========== TILE SIZE COMPUTATION ========== */

/**
 * Compute optimal blocking strategy for a convolution layer
 * 
 * @param h_out     Output height
 * @param w_out     Output width
 * @param c_in      Input channels
 * @param c_out     Output channels
 * @param k_h       Kernel height
 * @param k_w       Kernel width
 * @return          Allocated BlockingInfo with optimal tile sizes
 */
static inline BlockingInfo* compute_blocking_strategy(
    int h_out, int w_out, int c_in, int c_out, int k_h, int k_w) {
    
    BlockingInfo* info = (BlockingInfo*)calloc(1, sizeof(BlockingInfo));
    
    /* Special cases */
    info->is_pointwise = (k_h == 1 && k_w == 1);
    info->is_depthwise = false;  /* Detect this from layer properties later */
    
    /* Case 1: Output too small for blocking overhead */
    if (h_out * w_out < 64) {
        info->strategy = BLOCK_NONE;
        info->l1_tile_size = 0;
        return info;
    }
    
    /* Case 2: 1×1 convolution (pointwise) - use spatial tiling only */
    if (info->is_pointwise) {
        /* Working set: tile² × (c_in + c_out) × 4 bytes */
        int tile = (int)sqrt((double)L1_WORKING_SET / (4.0 * (c_in + c_out)));
        
        if (tile >= 128) info->l1_tile_size = 128;
        else if (tile >= 64) info->l1_tile_size = 64;
        else if (tile >= 32) info->l1_tile_size = 32;
        else if (tile >= 16) info->l1_tile_size = 16;
        else {
            /* Too many channels, need channel blocking instead (future work) */
            info->strategy = BLOCK_NONE;
            info->l1_tile_size = 0;
            return info;
        }
        
        info->strategy = BLOCK_L1;
        return info;
    }
    
    /* Case 3: Standard convolution - compute working set */
    /* Working set per tile:
     *   Input region: (tile + k_h - 1) × (tile + k_w - 1) × c_in × 4 bytes
     *   Output region: tile × tile × c_out × 4 bytes
     *   Kernel: k_h × k_w × c_in × c_out × 4 bytes (shared, ignore)
     */
    
    /* Try different tile sizes and find best fit for L1 */
    int best_tile = 0;
    int tile_candidates[] = {16, 8, 4};
    
    for (int i = 0; i < 3; i++) {
        int tile = tile_candidates[i];
        int input_region = (tile + k_h - 1) * (tile + k_w - 1) * c_in;
        int output_region = tile * tile * c_out;
        int working_set = (input_region + output_region) * 4;  /* 4 bytes per float */
        
        if (working_set <= L1_WORKING_SET) {
            best_tile = tile;
            break;
        }
    }
    
    /* Adjust for high channel counts (more conservative) */
    if (c_in + c_out > 256 && best_tile > 4) {
        best_tile /= 2;
    }
    
    if (best_tile > 0) {
        info->strategy = BLOCK_L1;
        info->l1_tile_size = best_tile;
        
        /* Check if we should add L2 blocking for large outputs */
        if (h_out * w_out > 4096) {  /* Output larger than 64×64 */
            info->strategy = BLOCK_L1_L2;
            info->l2_tile_size = 32;  /* 32×32 L2 tiles */
        }
    } else {
        /* No blocking possible */
        info->strategy = BLOCK_NONE;
        info->l1_tile_size = 0;
    }
    
    return info;
}

/**
 * Compute blocking for dense (fully connected) layers
 * Dense layers use different blocking: tile over batch and hidden dimensions
 * For inference (batch=1), blocking is less critical
 */
static inline BlockingInfo* compute_dense_blocking(int input_size, int output_size) {
    BlockingInfo* info = (BlockingInfo*)calloc(1, sizeof(BlockingInfo));
    
    /* For single-sample inference, dense layers are small enough
     * Matrix multiply blocking would help for batched inference (future) */
    info->strategy = BLOCK_NONE;
    info->l1_tile_size = 0;
    
    return info;
}

#endif /* NETLANG_BLOCKING_CONFIG_H */
