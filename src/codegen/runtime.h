/*
 * NetLang Runtime - Weight Loading and Memory Management
 * 
 * Provides high-performance weight loading using mmap for zero-copy access.
 * Supports .nwf (NetLang Weight Format) files with AVX2-aligned data.
 * 
 * Author: Abhijeet Deb Nath
 * Target: x86-64 with AVX2
 */

#ifndef NETLANG_RUNTIME_H
#define NETLANG_RUNTIME_H

#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
    #if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)
        #undef _WIN32_WINNT
        #define _WIN32_WINNT 0x0600
    #endif
    #if !defined(WINVER) || (WINVER < 0x0600)
        #undef WINVER
        #define WINVER 0x0600
    #endif
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

/* ========== WEIGHT FORMAT STRUCTURES ========== */

/* Cross-platform packing */
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
#pragma pack(push, 1)
#endif

/* File header (256 bytes) */
typedef struct {
    char magic[4];              /* "NWGT" */
    uint32_t version;           /* Format version */
    uint32_t layer_count;       /* Number of layers */
    uint64_t total_size;        /* File size in bytes */
    uint32_t alignment;         /* Data alignment (64 for AVX2) */
    uint32_t dtype;             /* 0=float32, 1=float16 */
    uint64_t metadata_offset;   /* Offset to metadata table */
    uint64_t data_offset;       /* Offset to weight data */
    uint8_t reserved[212];      /* Reserved for future use */
}
#ifdef __GNUC__
__attribute__((packed))
#endif
NWFHeader;

/* Layer metadata (64 bytes) */
typedef struct {
    uint32_t layer_type;        /* 0=Conv2D, 1=Dense, 2=BatchNorm */
    uint32_t layer_id;          /* Layer index */
    uint64_t weight_offset;     /* Byte offset to weights */
    uint64_t weight_size;       /* Weight data size */
    uint32_t weight_shape[4];   /* [dim0, dim1, dim2, dim3] */
    uint64_t bias_offset;       /* Byte offset to bias */
    uint64_t bias_size;         /* Bias data size */
    uint32_t bias_shape;        /* [dim0] */
    uint32_t reserved;          /* Padding to 64 bytes */
}
#ifdef __GNUC__
__attribute__((packed))
#endif
NWFLayerMeta;

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
#pragma pack(pop)
#endif

/* Weight file handle */
typedef struct {
    void* mapped_addr;          /* mmap address */
    uint64_t file_size;         /* Total file size */
    NWFHeader* header;          /* Pointer to header */
    NWFLayerMeta* metadata;     /* Pointer to metadata table */
    
#ifdef _WIN32
    HANDLE file_handle;
    HANDLE mapping_handle;
#else
    int fd;                     /* File descriptor (Unix) */
#endif
} WeightFile;

/* ========== TENSOR STRUCTURE ========== */

/* Generic tensor for inference */
typedef struct {
    float* data;                /* Data pointer (aligned) */
    int rank;                   /* Number of dimensions */
    int shape[4];               /* Dimensions [H, W, C] or [N] */
    int stride[4];              /* Strides for indexing */
    uint64_t size;              /* Total elements */
} Tensor;

/* ========== FUNCTION DECLARATIONS ========== */

typedef struct NetLangThreadPool NetLangThreadPool;
typedef void (*NetLangParallelRangeFn)(void* user_data, int start, int end);

/* Weight file operations */
WeightFile* load_weights(const char* path);
void unload_weights(WeightFile* wf);
float* get_layer_weights(WeightFile* wf, int layer_id);
float* get_layer_bias(WeightFile* wf, int layer_id);

/* Tensor operations */
Tensor* tensor_create(int rank, const int* shape);
void tensor_free(Tensor* t);
Tensor* tensor_create_from_weights(WeightFile* wf, int layer_id);

/* Memory alignment */
void* aligned_alloc_64(size_t size);
void aligned_free(void* ptr);

/* Runtime execution support */
double netlang_now_ms(void);
int netlang_default_thread_count(void);
int netlang_conv_spatial_block_override(void);
NetLangThreadPool* netlang_thread_pool_create(int requested_threads);
void netlang_thread_pool_destroy(NetLangThreadPool* pool);
int netlang_thread_pool_thread_count(const NetLangThreadPool* pool);
void netlang_parallel_for(NetLangThreadPool* pool,
                          int start,
                          int end,
                          int grain,
                          NetLangParallelRangeFn fn,
                          void* user_data);

#endif /* NETLANG_RUNTIME_H */
