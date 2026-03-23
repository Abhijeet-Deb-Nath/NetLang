/*
 * NetLang Benchmark Harness
 *
 * Measures init, first-inference, and warm in-process latency for a generated
 * network artifact. This is the current runtime benchmark path.
 *
 * Usage:
 *   benchmark.exe [--input input.bin] [--warmup N] [--runs N]
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../src/codegen/runtime.h"

typedef struct NetworkState NetworkState;
extern NetworkState* network_init(void);
extern void network_infer(NetworkState* net, float* input, float* output);
extern void network_cleanup(NetworkState* net);
extern const char* network_name(void);
extern size_t network_input_element_count(void);
extern size_t network_output_element_count(void);
extern size_t network_activation_arena_bytes(void);
extern int network_activation_slot_count(void);

#ifdef _WIN32
#include <windows.h>

static LARGE_INTEGER g_timer_frequency;

static void timer_init(void) {
    QueryPerformanceFrequency(&g_timer_frequency);
}

static double timer_now_ms(void) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)g_timer_frequency.QuadPart;
}
#else
#include <time.h>

static void timer_init(void) {
}

static double timer_now_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)now.tv_sec * 1000.0 + (double)now.tv_nsec / 1000000.0;
}
#endif

typedef struct BenchmarkConfig {
    const char* input_path;
    const char* json_output_path;
    const char* dump_output_path;
    int warmup_runs;
    int bench_runs;
} BenchmarkConfig;

typedef struct BenchmarkStats {
    double min_ms;
    double max_ms;
    double mean_ms;
    double median_ms;
    double p95_ms;
    double stddev_ms;
} BenchmarkStats;

static int compare_double(const void* lhs, const void* rhs) {
    double a = *(const double*)lhs;
    double b = *(const double*)rhs;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static long get_file_size_bytes(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return -1;
    }
    return (long)st.st_size;
}

static void print_usage(const char* program_name) {
    printf("NetLang Benchmark Harness\n");
    printf("\nUsage:\n");
    printf("  %s [--input input.bin] [--warmup N] [--runs N] [--json-out report.json] [--dump-output output.bin]\n",
           program_name);
    printf("\nExamples:\n");
    printf("  %s --input assets\\\\inputs\\\\preprocessed_28x28\\\\mnist_0000_label_7.bin\n", program_name);
    printf("  %s --warmup 25 --runs 500\n", program_name);
    printf("  %s --json-out results.json --dump-output output.bin\n", program_name);
}

static int parse_int_arg(const char* flag, const char* value) {
    char* end = NULL;
    long parsed = strtol(value, &end, 10);
    if (!value[0] || (end && *end != '\0') || parsed <= 0 || parsed > 1000000) {
        fprintf(stderr, "Error: Invalid value '%s' for %s\n", value, flag);
        return -1;
    }
    return (int)parsed;
}

static int parse_args(int argc, char** argv, BenchmarkConfig* config) {
    config->input_path = NULL;
    config->json_output_path = NULL;
    config->dump_output_path = NULL;
    config->warmup_runs = 20;
    config->bench_runs = 200;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --input requires a path\n");
                return 0;
            }
            config->input_path = argv[++i];
        } else if (strcmp(argv[i], "--json-out") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --json-out requires a path\n");
                return 0;
            }
            config->json_output_path = argv[++i];
        } else if (strcmp(argv[i], "--dump-output") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --dump-output requires a path\n");
                return 0;
            }
            config->dump_output_path = argv[++i];
        } else if (strcmp(argv[i], "--warmup") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --warmup requires a value\n");
                return 0;
            }
            config->warmup_runs = parse_int_arg("--warmup", argv[++i]);
            if (config->warmup_runs < 0) return 0;
        } else if (strcmp(argv[i], "--runs") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --runs requires a value\n");
                return 0;
            }
            config->bench_runs = parse_int_arg("--runs", argv[++i]);
            if (config->bench_runs < 0) return 0;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else {
            fprintf(stderr, "Error: Unknown argument '%s'\n", argv[i]);
            return 0;
        }
    }

    return 1;
}

static float* allocate_zero_input(size_t element_count) {
    float* input = (float*)aligned_alloc_64(element_count * sizeof(float));
    if (!input) {
        return NULL;
    }
    memset(input, 0, element_count * sizeof(float));
    return input;
}

static float* load_input_file(const char* path, size_t expected_elements) {
    long expected_bytes = (long)(expected_elements * sizeof(float));
    long file_size = get_file_size_bytes(path);
    if (file_size != expected_bytes) {
        fprintf(stderr, "Error: Input file size mismatch for '%s'\n", path);
        fprintf(stderr, "  Expected: %ld bytes (%zu floats)\n", expected_bytes, expected_elements);
        fprintf(stderr, "  Got: %ld bytes\n", file_size);
        return NULL;
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", path);
        return NULL;
    }

    float* input = (float*)aligned_alloc_64(expected_elements * sizeof(float));
    if (!input) {
        fclose(file);
        return NULL;
    }

    size_t read_count = fread(input, sizeof(float), expected_elements, file);
    fclose(file);

    if (read_count != expected_elements) {
        fprintf(stderr, "Error: Failed to read full input file '%s'\n", path);
        aligned_free(input);
        return NULL;
    }

    return input;
}

static double compute_output_checksum(const float* output, size_t element_count) {
    double checksum = 0.0;
    for (size_t i = 0; i < element_count; i++) {
        checksum += (double)output[i];
    }
    return checksum;
}

static int write_output_dump(const char* path, const float* output, size_t element_count) {
    FILE* file = fopen(path, "wb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open dump file '%s'\n", path);
        return 0;
    }

    size_t written = fwrite(output, sizeof(float), element_count, file);
    fclose(file);

    if (written != element_count) {
        fprintf(stderr, "Error: Failed to write complete dump file '%s'\n", path);
        return 0;
    }

    return 1;
}

static void write_json_string(FILE* file, const char* value) {
    fputc('"', file);
    for (const char* p = value; *p; p++) {
        if (*p == '\\' || *p == '"') {
            fputc('\\', file);
        }
        fputc(*p, file);
    }
    fputc('"', file);
}

static int write_json_report(const char* path,
                             const BenchmarkConfig* config,
                             const BenchmarkStats* stats,
                             double init_time_ms,
                             double first_inference_ms,
                             double output_checksum,
                             long exe_size_bytes,
                             size_t input_elements,
                             size_t output_elements) {
    FILE* file = fopen(path, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot open JSON report '%s'\n", path);
        return 0;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"network\": ");
    write_json_string(file, network_name());
    fprintf(file, ",\n");
    fprintf(file, "  \"input_elements\": %zu,\n", input_elements);
    fprintf(file, "  \"output_elements\": %zu,\n", output_elements);
    if (config->input_path) {
        fprintf(file, "  \"input_path\": ");
        write_json_string(file, config->input_path);
        fprintf(file, ",\n");
    } else {
        fprintf(file, "  \"input_path\": null,\n");
    }
    fprintf(file, "  \"warmup_runs\": %d,\n", config->warmup_runs);
    fprintf(file, "  \"bench_runs\": %d,\n", config->bench_runs);
    fprintf(file, "  \"activation_arena_bytes\": %zu,\n", network_activation_arena_bytes());
    fprintf(file, "  \"activation_slot_count\": %d,\n", network_activation_slot_count());
    fprintf(file, "  \"init_time_ms\": %.6f,\n", init_time_ms);
    fprintf(file, "  \"first_inference_ms\": %.6f,\n", first_inference_ms);
    fprintf(file, "  \"warm_mean_ms\": %.6f,\n", stats->mean_ms);
    fprintf(file, "  \"warm_median_ms\": %.6f,\n", stats->median_ms);
    fprintf(file, "  \"warm_p95_ms\": %.6f,\n", stats->p95_ms);
    fprintf(file, "  \"warm_min_ms\": %.6f,\n", stats->min_ms);
    fprintf(file, "  \"warm_max_ms\": %.6f,\n", stats->max_ms);
    fprintf(file, "  \"warm_stddev_ms\": %.6f,\n", stats->stddev_ms);
    fprintf(file, "  \"warm_throughput_ips\": %.6f,\n", stats->mean_ms > 0.0 ? 1000.0 / stats->mean_ms : 0.0);
    fprintf(file, "  \"output_checksum\": %.9f,\n", output_checksum);
    fprintf(file, "  \"executable_size_bytes\": %ld\n", exe_size_bytes);
    fprintf(file, "}\n");

    fclose(file);
    return 1;
}

static void calculate_stats(const double* samples, int sample_count, BenchmarkStats* stats) {
    double* sorted = (double*)malloc((size_t)sample_count * sizeof(double));
    double sum = 0.0;
    double variance_sum = 0.0;

    if (!sorted) {
        memset(stats, 0, sizeof(*stats));
        return;
    }

    memcpy(sorted, samples, (size_t)sample_count * sizeof(double));
    qsort(sorted, (size_t)sample_count, sizeof(double), compare_double);

    stats->min_ms = sorted[0];
    stats->max_ms = sorted[sample_count - 1];

    for (int i = 0; i < sample_count; i++) {
        sum += samples[i];
    }
    stats->mean_ms = sum / (double)sample_count;

    for (int i = 0; i < sample_count; i++) {
        double delta = samples[i] - stats->mean_ms;
        variance_sum += delta * delta;
    }
    stats->stddev_ms = sqrt(variance_sum / (double)sample_count);

    if ((sample_count % 2) == 0) {
        stats->median_ms = (sorted[sample_count / 2 - 1] + sorted[sample_count / 2]) / 2.0;
    } else {
        stats->median_ms = sorted[sample_count / 2];
    }

    int p95_index = (int)ceil(0.95 * (double)sample_count) - 1;
    if (p95_index < 0) p95_index = 0;
    if (p95_index >= sample_count) p95_index = sample_count - 1;
    stats->p95_ms = sorted[p95_index];

    free(sorted);
}

int main(int argc, char** argv) {
    BenchmarkConfig config;
    timer_init();

    if (!parse_args(argc, argv, &config)) {
        print_usage(argv[0]);
        return 1;
    }

    size_t input_elements = network_input_element_count();
    size_t output_elements = network_output_element_count();
    float* input = NULL;
    float* output = NULL;
    double* samples = NULL;

    printf("NetLang Benchmark Harness\n");
    printf("========================\n");
    printf("Network: %s\n", network_name());
    printf("Input elements: %zu\n", input_elements);
    printf("Output elements: %zu\n", output_elements);
    printf("Activation arena: %zu bytes\n", network_activation_arena_bytes());
    printf("Reusable slots: %d\n", network_activation_slot_count());
    printf("Warmup runs: %d\n", config.warmup_runs);
    printf("Measured runs: %d\n", config.bench_runs);

    if (config.input_path) {
        printf("Input mode: file (%s)\n", config.input_path);
        input = load_input_file(config.input_path, input_elements);
    } else {
        printf("Input mode: zero tensor\n");
        input = allocate_zero_input(input_elements);
    }

    if (!input) {
        fprintf(stderr, "Error: Failed to prepare input tensor\n");
        return 1;
    }

    output = (float*)aligned_alloc_64(output_elements * sizeof(float));
    samples = (double*)malloc((size_t)config.bench_runs * sizeof(double));
    if (!output || !samples) {
        fprintf(stderr, "Error: Failed to allocate benchmark buffers\n");
        aligned_free(input);
        aligned_free(output);
        free(samples);
        return 1;
    }

    long exe_size_bytes = get_file_size_bytes(argv[0]);

    double init_start_ms = timer_now_ms();
    NetworkState* net = network_init();
    double init_time_ms = timer_now_ms() - init_start_ms;
    if (!net) {
        fprintf(stderr, "Error: network_init() failed\n");
        aligned_free(input);
        aligned_free(output);
        free(samples);
        return 1;
    }

    double first_start_ms = timer_now_ms();
    network_infer(net, input, output);
    double first_inference_ms = timer_now_ms() - first_start_ms;

    for (int i = 0; i < config.warmup_runs; i++) {
        network_infer(net, input, output);
    }

    for (int i = 0; i < config.bench_runs; i++) {
        double start_ms = timer_now_ms();
        network_infer(net, input, output);
        samples[i] = timer_now_ms() - start_ms;
    }

    BenchmarkStats stats;
    double output_checksum = 0.0;
    calculate_stats(samples, config.bench_runs, &stats);
    output_checksum = compute_output_checksum(output, output_elements);

    printf("\nResults\n");
    printf("-------\n");
    printf("Init time: %.3f ms\n", init_time_ms);
    printf("First inference: %.3f ms\n", first_inference_ms);
    printf("Warm mean: %.3f ms\n", stats.mean_ms);
    printf("Warm median: %.3f ms\n", stats.median_ms);
    printf("Warm p95: %.3f ms\n", stats.p95_ms);
    printf("Warm min: %.3f ms\n", stats.min_ms);
    printf("Warm max: %.3f ms\n", stats.max_ms);
    printf("Warm stddev: %.3f ms\n", stats.stddev_ms);
    if (stats.mean_ms > 0.0) {
        printf("Warm throughput: %.2f inferences/sec\n", 1000.0 / stats.mean_ms);
    }
    printf("Output checksum: %.6f\n", output_checksum);
    if (exe_size_bytes >= 0) {
        printf("Executable size: %ld bytes\n", exe_size_bytes);
    }

    if (config.dump_output_path) {
        if (!write_output_dump(config.dump_output_path, output, output_elements)) {
            network_cleanup(net);
            aligned_free(input);
            aligned_free(output);
            free(samples);
            return 1;
        }
    }

    if (config.json_output_path) {
        if (!write_json_report(config.json_output_path,
                               &config,
                               &stats,
                               init_time_ms,
                               first_inference_ms,
                               output_checksum,
                               exe_size_bytes,
                               input_elements,
                               output_elements)) {
            network_cleanup(net);
            aligned_free(input);
            aligned_free(output);
            free(samples);
            return 1;
        }
    }

    network_cleanup(net);
    aligned_free(input);
    aligned_free(output);
    free(samples);

    return 0;
}
