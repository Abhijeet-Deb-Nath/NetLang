/*
 * Create Test Weight File
 * Generates a dummy .nwf file for testing the MNIST network
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    char magic[4];              /* "NWGT" */
    uint32_t version;           /* Format version */
    uint32_t layer_count;       /* Number of layers */
    uint64_t total_size;        /* File size in bytes */
    uint32_t alignment;         /* Data alignment (64 for AVX2) */
    uint32_t dtype;             /* 0=float32 */
    uint64_t metadata_offset;   /* Offset to metadata table */
    uint64_t data_offset;       /* Offset to weight data */
    uint8_t reserved[212];      /* Reserved for future use */
} NWFHeader;

typedef struct {
    uint32_t layer_type;        /* 0=conv2d, 1=dense */
    uint32_t layer_id;          /* Layer index */
    uint64_t weight_offset;     /* Offset (absolute) */
    uint64_t weight_size;       /* Size in bytes */
    uint32_t weight_shape[4];   /* Shape dimensions */
    uint64_t bias_offset;       /* Offset (absolute) */
    uint64_t bias_size;         /* Size in bytes */
    uint32_t bias_shape;        /* Bias dimension */
    uint32_t reserved;          /* Padding to 64 bytes */
} LayerMeta;
#pragma pack(pop)

/* Align offset to 64 bytes */
uint64_t align64(uint64_t offset) {
    return (offset + 63) & ~63ULL;
}

int main() {
    /* MNIST CNN layer sizes:
       Layer 0: Conv2D 32 filters, 3x3, input 1 channel -> weights: 3*3*1*32=288, bias: 32
       Layer 1: MaxPool (no weights)
       Layer 2: Conv2D 64 filters, 3x3, input 32 channels -> weights: 3*3*32*64=18432, bias: 64
       Layer 3: MaxPool (no weights)
       Layer 4: Flatten (no weights)
       Layer 5: Dense 128 units, input 3136 -> weights: 3136*128=401408, bias: 128
       Layer 6: Dense 10 units, input 128 -> weights: 128*10=1280, bias: 10
    */
    
    const int num_weight_layers = 4;  /* 2 Conv2D + 2 Dense */
    
    /* Calculate sizes */
    uint32_t conv1_weights = 3 * 3 * 1 * 32 * sizeof(float);   /* 1152 bytes */
    uint32_t conv1_bias = 32 * sizeof(float);                   /* 128 bytes */
    uint32_t conv2_weights = 3 * 3 * 32 * 64 * sizeof(float);  /* 73728 bytes */
    uint32_t conv2_bias = 64 * sizeof(float);                   /* 256 bytes */
    uint32_t dense1_weights = 3136 * 128 * sizeof(float);       /* 1605632 bytes */
    uint32_t dense1_bias = 128 * sizeof(float);                 /* 512 bytes */
    uint32_t dense2_weights = 128 * 10 * sizeof(float);         /* 5120 bytes */
    uint32_t dense2_bias = 10 * sizeof(float);                  /* 40 bytes */
    
    /* Header at offset 0 (256 bytes) */
    uint32_t header_size = sizeof(NWFHeader);  /* 256 bytes */
    
    /* Metadata starts at offset 256 */
    uint32_t metadata_offset = header_size;
    uint32_t metadata_size = num_weight_layers * sizeof(LayerMeta);
    
    /* Data starts at next 64-byte boundary after metadata */
    uint32_t data_offset = align64(metadata_offset + metadata_size);
    
    /* Calculate offsets for each layer's weights/bias (ABSOLUTE offsets) */
    uint64_t offset = data_offset;
    
    uint64_t conv1_w_off = offset; offset = align64(offset + conv1_weights);
    uint64_t conv1_b_off = offset; offset = align64(offset + conv1_bias);
    uint64_t conv2_w_off = offset; offset = align64(offset + conv2_weights);
    uint64_t conv2_b_off = offset; offset = align64(offset + conv2_bias);
    uint64_t dense1_w_off = offset; offset = align64(offset + dense1_weights);
    uint64_t dense1_b_off = offset; offset = align64(offset + dense1_bias);
    uint64_t dense2_w_off = offset; offset = align64(offset + dense2_weights);
    uint64_t dense2_b_off = offset; offset = align64(offset + dense2_bias);
    
    uint32_t total_data_size = offset - data_offset;
    uint64_t total_file_size = offset;
    
    printf("Creating test weight file...\n");
    printf("  Header: %u bytes\n", header_size);
    printf("  Metadata: %u bytes (%d layers)\n", metadata_size, num_weight_layers);
    printf("  Data offset: %u\n", data_offset);
    printf("  Total data: %u bytes\n", total_data_size);
    printf("  Total file: %llu bytes\n", (unsigned long long)total_file_size);
    
    /* Create file */
    FILE* f = fopen("models/mnist_cnn.nwf", "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot create weight file\n");
        return 1;
    }
    
    /* Write header */
    NWFHeader header = {0};
    memcpy(header.magic, "NWGT", 4);
    header.version = 1;
    header.layer_count = num_weight_layers;
    header.total_size = total_file_size;
    header.alignment = 64;
    header.dtype = 0;  /* float32 */
    header.metadata_offset = metadata_offset;
    header.data_offset = data_offset;
    
    fwrite(&header, sizeof(header), 1, f);
    
    /* Write metadata */
    LayerMeta layers[4] = {0};
    
    /* Conv2D layer 0 */
    layers[0].layer_type = 0;  /* conv2d */
    layers[0].layer_id = 0;
    layers[0].weight_offset = conv1_w_off;
    layers[0].weight_size = conv1_weights;
    layers[0].weight_shape[0] = 32;  /* filters */
    layers[0].weight_shape[1] = 1;   /* in_channels */
    layers[0].weight_shape[2] = 3;   /* kernel_h */
    layers[0].weight_shape[3] = 3;   /* kernel_w */
    layers[0].bias_offset = conv1_b_off;
    layers[0].bias_size = conv1_bias;
    layers[0].bias_shape = 32;
    
    /* Conv2D layer 2 */
    layers[1].layer_type = 0;  /* conv2d */
    layers[1].layer_id = 1;
    layers[1].weight_offset = conv2_w_off;
    layers[1].weight_size = conv2_weights;
    layers[1].weight_shape[0] = 64;
    layers[1].weight_shape[1] = 32;
    layers[1].weight_shape[2] = 3;
    layers[1].weight_shape[3] = 3;
    layers[1].bias_offset = conv2_b_off;
    layers[1].bias_size = conv2_bias;
    layers[1].bias_shape = 64;
    
    /* Dense layer 5 */
    layers[2].layer_type = 1;  /* dense */
    layers[2].layer_id = 2;
    layers[2].weight_offset = dense1_w_off;
    layers[2].weight_size = dense1_weights;
    layers[2].weight_shape[0] = 128;   /* out_features */
    layers[2].weight_shape[1] = 3136;  /* in_features */
    layers[2].weight_shape[2] = 1;
    layers[2].weight_shape[3] = 1;
    layers[2].bias_offset = dense1_b_off;
    layers[2].bias_size = dense1_bias;
    layers[2].bias_shape = 128;
    
    /* Dense layer 6 */
    layers[3].layer_type = 1;  /* dense */
    layers[3].layer_id = 3;
    layers[3].weight_offset = dense2_w_off;
    layers[3].weight_size = dense2_weights;
    layers[3].weight_shape[0] = 10;
    layers[3].weight_shape[1] = 128;
    layers[3].weight_shape[2] = 1;
    layers[3].weight_shape[3] = 1;
    layers[3].bias_offset = dense2_b_off;
    layers[3].bias_size = dense2_bias;
    layers[3].bias_shape = 10;
    
    fwrite(layers, sizeof(LayerMeta), 4, f);
    
    /* Pad to data_offset */
    long current_pos = metadata_offset + metadata_size;
    while (current_pos < (long)data_offset) {
        fputc(0, f);
        current_pos++;
    }
    
    /* Write weight data for each layer with alignment padding */
    srand(42);  /* Reproducible results */
    
    /* Helper macro to write random floats */
    #define WRITE_RANDOM_DATA(count) do { \
        for (uint32_t i = 0; i < (count); i++) { \
            float val = ((float)rand() / RAND_MAX - 0.5f) * 0.1f; \
            fwrite(&val, sizeof(float), 1, f); \
        } \
    } while(0)
    
    #define PAD_TO(offset) do { \
        long cur = ftell(f); \
        while (cur < (long)(offset)) { fputc(0, f); cur++; } \
    } while(0)
    
    /* Conv1 weights */
    PAD_TO(conv1_w_off);
    WRITE_RANDOM_DATA(conv1_weights / sizeof(float));
    /* Conv1 bias */
    PAD_TO(conv1_b_off);
    WRITE_RANDOM_DATA(conv1_bias / sizeof(float));
    
    /* Conv2 weights */
    PAD_TO(conv2_w_off);
    WRITE_RANDOM_DATA(conv2_weights / sizeof(float));
    /* Conv2 bias */
    PAD_TO(conv2_b_off);
    WRITE_RANDOM_DATA(conv2_bias / sizeof(float));
    
    /* Dense1 weights */
    PAD_TO(dense1_w_off);
    WRITE_RANDOM_DATA(dense1_weights / sizeof(float));
    /* Dense1 bias */
    PAD_TO(dense1_b_off);
    WRITE_RANDOM_DATA(dense1_bias / sizeof(float));
    
    /* Dense2 weights */
    PAD_TO(dense2_w_off);
    WRITE_RANDOM_DATA(dense2_weights / sizeof(float));
    /* Dense2 bias */
    PAD_TO(dense2_b_off);
    WRITE_RANDOM_DATA(dense2_bias / sizeof(float));
    
    fclose(f);
    printf("Created: models/mnist_cnn.nwf (%llu bytes)\n", (unsigned long long)total_file_size);
    
    return 0;
}
