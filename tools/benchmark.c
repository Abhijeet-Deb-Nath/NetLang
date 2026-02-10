/*
 * NetLang Inference Benchmark
 * 
 * Measures inference time and throughput for compiled NetLang networks.
 * Compares against baseline implementations.
 * 
 * Usage: ./benchmark <network_binary> <test_image> <iterations>
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../src/codegen/runtime.h"
#include "../src/codegen/kernels.h"

#ifdef _WIN32
    #include <windows.h>
    #define GET_TIME() GetTickCount64()
    #define TIME_UNIT "ms"
#else
    #include <sys/time.h>
    static inline unsigned long long GET_TIME() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000ULL + tv.tv_usec;
    }
    #define TIME_UNIT "µs"
#endif

/* Statistics structure */
typedef struct {
    double min_time;
    double max_time;
    double avg_time;
    double std_dev;
    double median_time;
    int total_runs;
} BenchmarkStats;

/* Calculate statistics from timing samples */
void calculate_stats(double* times, int n, BenchmarkStats* stats) {
    stats->total_runs = n;
    stats->min_time = times[0];
    stats->max_time = times[0];
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        if (times[i] < stats->min_time) stats->min_time = times[i];
        if (times[i] > stats->max_time) stats->max_time = times[i];
        sum += times[i];
    }
    
    stats->avg_time = sum / n;
    
    /* Standard deviation */
    double var_sum = 0.0;
    for (int i = 0; i < n; i++) {
        double diff = times[i] - stats->avg_time;
        var_sum += diff * diff;
    }
    stats->std_dev = sqrt(var_sum / n);
    
    /* Median (sort times first) */
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (times[j] < times[i]) {
                double temp = times[i];
                times[i] = times[j];
                times[j] = temp;
            }
        }
    }
    stats->median_time = (n % 2 == 0) ? 
        (times[n/2 - 1] + times[n/2]) / 2.0 : times[n/2];
}

/* Print benchmark results */
void print_results(const char* name, BenchmarkStats* stats) {
    printf("\n");
    printf("=================================================\n");
    printf(" Benchmark Results: %s\n", name);
    printf("=================================================\n");
    printf("  Total runs:        %d\n", stats->total_runs);
    printf("  Min time:          %.3f ms\n", stats->min_time);
    printf("  Max time:          %.3f ms\n", stats->max_time);
    printf("  Average time:      %.3f ms\n", stats->avg_time);
    printf("  Median time:       %.3f ms\n", stats->median_time);
    printf("  Std deviation:     %.3f ms\n", stats->std_dev);
    printf("  Throughput:        %.2f images/sec\n", 1000.0 / stats->avg_time);
    printf("=================================================\n");
}

/* Example benchmark for MNIST CNN */
void benchmark_mnist_example(int warmup_runs, int bench_runs) {
    printf("Benchmarking MNIST CNN (example)...\n");
    
    /* Allocate timing array */
    double* times = (double*)malloc(bench_runs * sizeof(double));
    
    /* Create dummy input */
    float* input = (float*)aligned_alloc_64(28 * 28 * 1 * sizeof(float));
    for (int i = 0; i < 28 * 28; i++) {
        input[i] = (float)rand() / RAND_MAX;  /* Random values */
    }
    
    float* output = (float*)aligned_alloc_64(10 * sizeof(float));
    
    /* Load weights */
    WeightFile* wf = load_weights("models/mnist.nwf");
    if (!wf) {
        fprintf(stderr, "Error: Cannot load weights\n");
        return;
    }
    
    printf("Warmup: %d runs...\n", warmup_runs);
    for (int i = 0; i < warmup_runs; i++) {
        /* Dummy inference - replace with actual network call */
        memset(output, 0, 10 * sizeof(float));
    }
    
    printf("Benchmark: %d runs...\n", bench_runs);
    for (int i = 0; i < bench_runs; i++) {
        unsigned long long start = GET_TIME();
        
        /* ===== ACTUAL INFERENCE HERE ===== */
        /* Replace with generated code:
         * mnist_cnn_infer(input, output);
         */
        memset(output, 0, 10 * sizeof(float));  /* Placeholder */
        
        unsigned long long end = GET_TIME();
        
        /* Convert to milliseconds */
#ifdef _WIN32
        times[i] = (double)(end - start);
#else
        times[i] = (double)(end - start) / 1000.0;
#endif
    }
    
    /* Calculate and print statistics */
    BenchmarkStats stats;
    calculate_stats(times, bench_runs, &stats);
    print_results("MNIST CNN", &stats);
    
    /* Cleanup */
    unload_weights(wf);
    aligned_free(input);
    aligned_free(output);
    free(times);
}

/* Main benchmark driver */
int main(int argc, char** argv) {
    printf("NetLang Inference Benchmark\n");
    printf("Target: x86-64 AVX2 (Intel Core i5-5200U)\n\n");
    
    int warmup_runs = 10;
    int bench_runs = 100;
    
    if (argc > 1) {
        warmup_runs = atoi(argv[1]);
    }
    if (argc > 2) {
        bench_runs = atoi(argv[2]);
    }
    
    /* Run benchmarks */
    benchmark_mnist_example(warmup_runs, bench_runs);
    
    /* Add more benchmarks here:
     * benchmark_vgg16(warmup_runs, bench_runs);
     * benchmark_resnet50(warmup_runs, bench_runs);
     */
    
    return 0;
}
