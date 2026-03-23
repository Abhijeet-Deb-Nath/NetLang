#!/usr/bin/env python3
import struct
import sys

def check_nwf(filepath):
    data = open(filepath, 'rb').read()
    
    print('=== File Analysis ===')
    print(f'File size: {len(data)} bytes')
    
    # Header
    print('\n=== Header ===')
    print(f'  Magic: {data[0:4]}')
    print(f'  Version: {struct.unpack("<I", data[4:8])[0]}')
    print(f'  Layer count: {struct.unpack("<I", data[8:12])[0]}')
    print(f'  Total size: {struct.unpack("<Q", data[12:20])[0]}')
    print(f'  Alignment: {struct.unpack("<I", data[20:24])[0]}')
    print(f'  Dtype: {struct.unpack("<I", data[24:28])[0]}')
    print(f'  Metadata offset: {struct.unpack("<Q", data[28:36])[0]}')
    print(f'  Data offset: {struct.unpack("<Q", data[36:44])[0]}')
    
    # Read first layer metadata
    meta_offset = struct.unpack('<Q', data[28:36])[0]
    print(f'\n=== First Layer Metadata (at offset {meta_offset}) ===')
    layer_type = struct.unpack('<I', data[meta_offset:meta_offset+4])[0]
    layer_id = struct.unpack('<I', data[meta_offset+4:meta_offset+8])[0]
    weight_offset = struct.unpack('<Q', data[meta_offset+8:meta_offset+16])[0]
    weight_size = struct.unpack('<Q', data[meta_offset+16:meta_offset+24])[0]
    print(f'  Layer type: {layer_type}')
    print(f'  Layer id: {layer_id}')
    print(f'  Weight offset: {weight_offset}')
    print(f'  Weight size: {weight_size}')
    
    # Read weights at the documented offset
    print(f'\n=== Weight data at offset {weight_offset} ===')
    weights = struct.unpack('<10f', data[weight_offset:weight_offset+40])
    print(f'  First 10 weights: {weights}')

if __name__ == '__main__':
    check_nwf(sys.argv[1] if len(sys.argv) > 1 else 'assets/weights/netlang/lenet5_trained.nwf')
