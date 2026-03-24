/*
 * NetLang Runtime Implementation
 * 
 * High-performance weight loading using mmap for zero-copy access.
 * Cross-platform support (Windows/Linux).
 */

#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef _WIN32
#include <malloc.h>  /* For _aligned_malloc/_aligned_free */
#else
#include <time.h>
#endif

struct NetLangThreadPool {
    int thread_count;
#ifdef _WIN32
    int worker_count;
    HANDLE* threads;
    CONDITION_VARIABLE cv;
    SRWLOCK lock;
    int shutdown;
    unsigned long generation;
    int next_begin;
    int end;
    int grain;
    int active_participants;
    NetLangParallelRangeFn fn;
    void* user_data;
#endif
};

#ifdef _WIN32
static void thread_pool_run_job(NetLangThreadPool* pool) {
    for (;;) {
        NetLangParallelRangeFn fn = NULL;
        void* user_data = NULL;
        int begin = 0;
        int end = 0;

        AcquireSRWLockExclusive(&pool->lock);
        if (pool->next_begin >= pool->end) {
            pool->active_participants--;
            if (pool->active_participants == 0) {
                WakeAllConditionVariable(&pool->cv);
            }
            ReleaseSRWLockExclusive(&pool->lock);
            return;
        }

        begin = pool->next_begin;
        end = begin + pool->grain;
        if (end > pool->end) {
            end = pool->end;
        }
        pool->next_begin = end;
        fn = pool->fn;
        user_data = pool->user_data;
        ReleaseSRWLockExclusive(&pool->lock);

        fn(user_data, begin, end);
    }
}

static DWORD WINAPI thread_pool_worker(LPVOID param) {
    NetLangThreadPool* pool = (NetLangThreadPool*)param;
    unsigned long observed_generation = 0;

    AcquireSRWLockExclusive(&pool->lock);
    for (;;) {
        while (!pool->shutdown && pool->generation == observed_generation) {
            SleepConditionVariableSRW(&pool->cv, &pool->lock, INFINITE, 0);
        }

        if (pool->shutdown) {
            break;
        }

        observed_generation = pool->generation;
        ReleaseSRWLockExclusive(&pool->lock);
        thread_pool_run_job(pool);
        AcquireSRWLockExclusive(&pool->lock);
    }
    ReleaseSRWLockExclusive(&pool->lock);

    return 0;
}
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

/* ========== EXECUTION SUPPORT ========== */

double netlang_now_ms(void) {
#ifdef _WIN32
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER now;

    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }

    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)frequency.QuadPart;
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)now.tv_sec * 1000.0 + (double)now.tv_nsec / 1000000.0;
#endif
}

int netlang_default_thread_count(void) {
    int thread_count = 1;
    const char* env_value = getenv("NETLANG_THREADS");

    if (env_value && env_value[0]) {
        char* end = NULL;
        long parsed = strtol(env_value, &end, 10);
        if (end && *end == '\0' && parsed > 0 && parsed <= INT_MAX) {
            return (int)parsed;
        }
    }

#ifdef _WIN32
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        if (info.dwNumberOfProcessors > 0) {
            thread_count = (int)info.dwNumberOfProcessors;
        }
    }
#else
    {
        long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
        if (cpu_count > 0 && cpu_count <= INT_MAX) {
            thread_count = (int)cpu_count;
        }
    }
#endif

    return thread_count > 0 ? thread_count : 1;
}

NetLangThreadPool* netlang_thread_pool_create(int requested_threads) {
    NetLangThreadPool* pool = calloc(1, sizeof(NetLangThreadPool));
    if (!pool) {
        return NULL;
    }

    pool->thread_count = requested_threads > 0 ? requested_threads : netlang_default_thread_count();
    if (pool->thread_count < 1) {
        pool->thread_count = 1;
    }

#ifdef _WIN32
    pool->worker_count = pool->thread_count > 1 ? pool->thread_count - 1 : 0;
    InitializeConditionVariable(&pool->cv);
    InitializeSRWLock(&pool->lock);

    if (pool->worker_count > 0) {
        pool->threads = calloc((size_t)pool->worker_count, sizeof(HANDLE));
        if (!pool->threads) {
            pool->thread_count = 1;
            pool->worker_count = 0;
            return pool;
        }

        for (int i = 0; i < pool->worker_count; i++) {
            pool->threads[i] = CreateThread(NULL, 0, thread_pool_worker, pool, 0, NULL);
            if (!pool->threads[i]) {
                pool->shutdown = 1;
                WakeAllConditionVariable(&pool->cv);
                for (int j = 0; j < i; j++) {
                    WaitForSingleObject(pool->threads[j], INFINITE);
                    CloseHandle(pool->threads[j]);
                }
                free(pool->threads);
                pool->threads = NULL;
                pool->thread_count = 1;
                pool->worker_count = 0;
                pool->shutdown = 0;
                break;
            }
        }
    }
#endif

    return pool;
}

void netlang_thread_pool_destroy(NetLangThreadPool* pool) {
    if (!pool) {
        return;
    }

#ifdef _WIN32
    if (pool->worker_count > 0 && pool->threads) {
        AcquireSRWLockExclusive(&pool->lock);
        pool->shutdown = 1;
        WakeAllConditionVariable(&pool->cv);
        ReleaseSRWLockExclusive(&pool->lock);

        WaitForMultipleObjects((DWORD)pool->worker_count, pool->threads, TRUE, INFINITE);
        for (int i = 0; i < pool->worker_count; i++) {
            CloseHandle(pool->threads[i]);
        }
        free(pool->threads);
    }
#endif

    free(pool);
}

int netlang_thread_pool_thread_count(const NetLangThreadPool* pool) {
    if (!pool) {
        return 1;
    }

    return pool->thread_count > 0 ? pool->thread_count : 1;
}

void netlang_parallel_for(NetLangThreadPool* pool,
                          int start,
                          int end,
                          int grain,
                          NetLangParallelRangeFn fn,
                          void* user_data) {
    if (!fn || start >= end) {
        return;
    }

    if (!pool || pool->thread_count <= 1) {
        fn(user_data, start, end);
        return;
    }

#ifdef _WIN32
    if (grain <= 0) {
        grain = 1;
    }

    if ((end - start) <= grain || pool->worker_count <= 0) {
        fn(user_data, start, end);
        return;
    }

    AcquireSRWLockExclusive(&pool->lock);
    pool->fn = fn;
    pool->user_data = user_data;
    pool->next_begin = start;
    pool->end = end;
    pool->grain = grain;
    pool->active_participants = pool->worker_count + 1;
    pool->generation++;
    WakeAllConditionVariable(&pool->cv);
    ReleaseSRWLockExclusive(&pool->lock);

    thread_pool_run_job(pool);

    AcquireSRWLockExclusive(&pool->lock);
    while (pool->active_participants > 0) {
        SleepConditionVariableSRW(&pool->cv, &pool->lock, INFINITE, 0);
    }
    pool->fn = NULL;
    pool->user_data = NULL;
    ReleaseSRWLockExclusive(&pool->lock);
#else
    (void)grain;
    fn(user_data, start, end);
#endif
}
