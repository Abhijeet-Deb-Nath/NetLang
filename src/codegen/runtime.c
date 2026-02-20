/*
 * NetLang Runtime Implementation
 * 
 * High-performance weight loading using mmap for zero-copy access.
 * Cross-platform support (Windows/Linux).
 */

#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <malloc.h>  /* For _aligned_malloc/_aligned_free */
#endif

/* ========== WEIGHT FILE OPERATIONS ========== */

WeightFile* load_weights(const char* path) {
    WeightFile* wf = (WeightFile*)calloc(1, sizeof(WeightFile));
    if (!wf) {
        fprintf(stderr, "Error: Failed to allocate WeightFile\n");
        return NULL;
    }
    
#ifdef _WIN32
    /* Windows implementation using CreateFileMapping */
    wf->file_handle = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (wf->file_handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Cannot open weight file: %s\n", path);
        free(wf);
        return NULL;
    }
    
    /* Get file size */
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(wf->file_handle, &file_size)) {
        fprintf(stderr, "Error: Cannot get file size\n");
        CloseHandle(wf->file_handle);
        free(wf);
        return NULL;
    }
    wf->file_size = file_size.QuadPart;
    
    /* Create file mapping */
    wf->mapping_handle = CreateFileMappingA(
        wf->file_handle,
        NULL,
        PAGE_READONLY,
        0, 0,  /* Map entire file */
        NULL
    );
    
    if (!wf->mapping_handle) {
        fprintf(stderr, "Error: Cannot create file mapping\n");
        CloseHandle(wf->file_handle);
        free(wf);
        return NULL;
    }
    
    /* Map into memory */
    wf->mapped_addr = MapViewOfFile(
        wf->mapping_handle,
        FILE_MAP_READ,
        0, 0, 0  /* Map entire file */
    );
    
    if (!wf->mapped_addr) {
        fprintf(stderr, "Error: Cannot map view of file\n");
        CloseHandle(wf->mapping_handle);
        CloseHandle(wf->file_handle);
        free(wf);
        return NULL;
    }
    
#else
    /* Unix/Linux implementation using mmap */
    wf->fd = open(path, O_RDONLY);
    if (wf->fd < 0) {
        fprintf(stderr, "Error: Cannot open weight file: %s\n", path);
        free(wf);
        return NULL;
    }
    
    /* Get file size */
    struct stat sb;
    if (fstat(wf->fd, &sb) < 0) {
        fprintf(stderr, "Error: Cannot stat file\n");
        close(wf->fd);
        free(wf);
        return NULL;
    }
    wf->file_size = sb.st_size;
    
    /* Memory map the file */
    wf->mapped_addr = mmap(
        NULL,                   /* Let kernel choose address */
        wf->file_size,          /* Map entire file */
        PROT_READ,              /* Read-only */
        MAP_PRIVATE,            /* Private mapping (copy-on-write) */
        wf->fd,                 /* File descriptor */
        0                       /* Offset */
    );
    
    if (wf->mapped_addr == MAP_FAILED) {
        fprintf(stderr, "Error: Cannot mmap file\n");
        close(wf->fd);
        free(wf);
        return NULL;
    }
    
    /* Advise kernel about access pattern for optimization */
    madvise(wf->mapped_addr, wf->file_size, 
            MADV_SEQUENTIAL | MADV_WILLNEED);
#endif
    
    /* Set up pointers to header and metadata */
    wf->header = (NWFHeader*)wf->mapped_addr;
    
    /* Validate magic number */
    if (memcmp(wf->header->magic, "NWGT", 4) != 0) {
        fprintf(stderr, "Error: Invalid weight file format (bad magic number)\n");
        unload_weights(wf);
        return NULL;
    }
    
    /* Validate version */
    if (wf->header->version != 1) {
        fprintf(stderr, "Error: Unsupported weight file version: %u\n", 
                wf->header->version);
        unload_weights(wf);
        return NULL;
    }
    
    /* Set metadata pointer */
    wf->metadata = (NWFLayerMeta*)((uint8_t*)wf->mapped_addr + 
                                   wf->header->metadata_offset);
    
    return wf;
}

void unload_weights(WeightFile* wf) {
    if (!wf) return;
    
#ifdef _WIN32
    if (wf->mapped_addr) {
        UnmapViewOfFile(wf->mapped_addr);
    }
    if (wf->mapping_handle) {
        CloseHandle(wf->mapping_handle);
    }
    if (wf->file_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(wf->file_handle);
    }
#else
    if (wf->mapped_addr && wf->mapped_addr != MAP_FAILED) {
        munmap(wf->mapped_addr, wf->file_size);
    }
    if (wf->fd >= 0) {
        close(wf->fd);
    }
#endif
    
    free(wf);
}

float* get_layer_weights(WeightFile* wf, int layer_id) {
    if (!wf || layer_id < 0 || layer_id >= (int)wf->header->layer_count) {
        return NULL;
    }
    
    NWFLayerMeta* meta = &wf->metadata[layer_id];
    return (float*)((uint8_t*)wf->mapped_addr + meta->weight_offset);
}

float* get_layer_bias(WeightFile* wf, int layer_id) {
    if (!wf || layer_id < 0 || layer_id >= (int)wf->header->layer_count) {
        return NULL;
    }
    
    NWFLayerMeta* meta = &wf->metadata[layer_id];
    if (meta->bias_offset == 0) {
        return NULL;  /* No bias */
    }
    
    return (float*)((uint8_t*)wf->mapped_addr + meta->bias_offset);
}

/* ========== TENSOR OPERATIONS ========== */

Tensor* tensor_create(int rank, const int* shape) {
    Tensor* t = (Tensor*)calloc(1, sizeof(Tensor));
    if (!t) return NULL;
    
    t->rank = rank;
    t->size = 1;
    
    for (int i = 0; i < rank; i++) {
        t->shape[i] = shape[i];
        t->size *= shape[i];
    }
    
    /* Calculate strides (row-major) */
    t->stride[rank - 1] = 1;
    for (int i = rank - 2; i >= 0; i--) {
        t->stride[i] = t->stride[i + 1] * t->shape[i + 1];
    }
    
    /* Allocate aligned memory */
    t->data = (float*)aligned_alloc_64(t->size * sizeof(float));
    if (!t->data) {
        free(t);
        return NULL;
    }
    
    return t;
}

void tensor_free(Tensor* t) {
    if (!t) return;
    if (t->data) {
        aligned_free(t->data);
    }
    free(t);
}

/* ========== ALIGNED MEMORY ALLOCATION ========== */

void* aligned_alloc_64(size_t size) {
#ifdef _WIN32
    #ifdef __MINGW32__
        return __mingw_aligned_malloc(size, 64);
    #else
        return _aligned_malloc(size, 64);
    #endif
#else
    void* ptr = NULL;
    if (posix_memalign(&ptr, 64, size) != 0) {
        return NULL;
    }
    return ptr;
#endif
}

void aligned_free(void* ptr) {
    if (!ptr) return;
    
#ifdef _WIN32
    #ifdef __MINGW32__
        __mingw_aligned_free(ptr);
    #else
        _aligned_free(ptr);
    #endif
#else
    free(ptr);
#endif
}
