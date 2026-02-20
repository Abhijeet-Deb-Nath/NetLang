#!/usr/bin/env python3
"""Batch accuracy test for NetLang compiled network"""
import subprocess
import sys
import os
import re
from pathlib import Path

def main():
    test_dir = Path('test_data/preprocessed_28x28')
    test_files = sorted(test_dir.glob('mnist_*.bin'))
    
    n_test = int(sys.argv[1]) if len(sys.argv) > 1 else 100
    test_files = test_files[:n_test]
    
    print(f"Testing {len(test_files)} images...")
    
    correct = 0
    total = 0
    
    for f in test_files:
        # Extract label from filename
        match = re.search(r'label_(\d+)', f.name)
        if not match:
            continue
        
        true_label = int(match.group(1))
        
        # Run inference
        result = subprocess.run(
            ['bin\\test_lenet5.exe', str(f)],
            capture_output=True,
            text=True
        )
        
        # Extract predicted class
        pred_match = re.search(r'RESULT:\s*(\d+)', result.stdout)
        if not pred_match:
            print(f"ERROR: {f.name} - could not parse output")
            continue
        
        pred_label = int(pred_match.group(1))
        
        total += 1
        if pred_label == true_label:
            correct += 1
            status = "✓"
        else:
            status = "✗"
        
        # Print progress every 10
        if total % 10 == 0 or total == len(test_files):
            acc = 100 * correct / total
            print(f"  [{total}/{len(test_files)}] Accuracy: {acc:.1f}% ({correct}/{total})")
    
    final_acc = 100 * correct / total
    print(f"\n{'='*50}")
    print(f"Final Accuracy: {final_acc:.2f}% ({correct}/{total})")
    print(f"{'='*50}")

if __name__ == '__main__':
    main()
