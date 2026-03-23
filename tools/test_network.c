/*
 * NetLang Network Test Harness
 *
 * Reads a preprocessed .bin file and runs inference using generated network code.
 *
 * This program is for smoke testing and local correctness checks.
 * It is not the final benchmark harness for the research direction.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

/* Network interface (from generated code) */
typedef struct NetworkState NetworkState;
extern NetworkState* network_init(void);
extern void network_infer(NetworkState* net, float* input, float* output);
extern void network_cleanup(NetworkState* net);
extern size_t network_activation_arena_bytes(void);
extern int network_activation_slot_count(void);

/* Expected input size for the current MNIST smoke path */
#define INPUT_SIZE (28 * 28 * 1)
#define OUTPUT_SIZE 10

/* Helper: Get file size */
long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

/* Helper: Load binary file */
float* load_bin_file(const char* filename, size_t expected_size) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    long file_size = get_file_size(filename);
    long expected_bytes = (long)(expected_size * sizeof(float));

    if (file_size != expected_bytes) {
        fprintf(stderr, "Error: File size mismatch\n");
        fprintf(stderr, "  Expected: %ld bytes (%zu floats)\n", expected_bytes, expected_size);
        fprintf(stderr, "  Got: %ld bytes\n", file_size);
        fprintf(stderr, "  (Did you preprocess with tools/preprocess.py?)\n");
        fclose(f);
        return NULL;
    }

    float* data = (float*)malloc(expected_size * sizeof(float));
    if (!data) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(f);
        return NULL;
    }

    size_t read = fread(data, sizeof(float), expected_size, f);
    fclose(f);

    if (read != expected_size) {
        fprintf(stderr, "Error: Read %zu floats, expected %zu\n", read, expected_size);
        free(data);
        return NULL;
    }

    return data;
}

/* Helper: Find argmax (predicted class) */
int argmax(const float* arr, int size) {
    int max_idx = 0;
    float max_val = arr[0];

    for (int i = 1; i < size; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
            max_idx = i;
        }
    }

    return max_idx;
}

/* Helper: Print probability distribution */
void print_probabilities(const float* probs, int size) {
    printf("\nProbability distribution:\n");
    for (int i = 0; i < size; i++) {
        int bar_width = (int)(probs[i] * 50.0f);
        printf("  Class %d: %5.2f%% |", i, probs[i] * 100.0f);
        for (int j = 0; j < bar_width; j++) {
            printf("#");
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("NetLang Network Test Harness\n");
        printf("\nUsage: %s <input.bin>\n", argv[0]);
        printf("\nExample:\n");
        printf("  %s assets/inputs/preprocessed_28x28/mnist_0000_label_7.bin\n", argv[0]);
        printf("\nNote: Input must be preprocessed with tools/preprocess.py\n");
        return 1;
    }

    const char* input_file = argv[1];

    printf("NetLang Inference\n");
    printf("=================\n");
    printf("Input file: %s\n", input_file);

    printf("Loading input... ");
    fflush(stdout);
    float* input = load_bin_file(input_file, INPUT_SIZE);
    if (!input) {
        return 1;
    }
    printf("OK (%d floats)\n", INPUT_SIZE);

    printf("Initializing network... ");
    fflush(stdout);
    NetworkState* net = network_init();
    if (!net) {
        free(input);
        return 1;
    }
    printf("OK\n");
    printf("Planner arena: %zu bytes across %d reusable slots\n",
           network_activation_arena_bytes(),
           network_activation_slot_count());

    printf("Allocating output buffer... ");
    fflush(stdout);
    float* output = (float*)malloc(OUTPUT_SIZE * sizeof(float));
    if (!output) {
        fprintf(stderr, "Error: Cannot allocate output buffer\n");
        network_cleanup(net);
        free(input);
        return 1;
    }
    printf("OK\n");

    printf("Running inference... ");
    fflush(stdout);
    clock_t start = clock();
    network_infer(net, input, output);
    clock_t end = clock();

    double inference_time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    printf("OK (%.2f ms)\n", inference_time_ms);

    int predicted = argmax(output, OUTPUT_SIZE);
    float confidence = output[predicted];

    printf("\n");
    printf("=================\n");
    printf("RESULT: %d\n", predicted);
    printf("=================\n");
    printf("Confidence: %.2f%%\n", confidence * 100.0f);
    printf("Inference time: %.2f ms\n", inference_time_ms);

    print_probabilities(output, OUTPUT_SIZE);

    free(input);
    free(output);
    network_cleanup(net);

    printf("\nDone!\n");
    return 0;
}
