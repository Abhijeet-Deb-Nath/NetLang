#!/usr/bin/env python3
"""
Verify Model Correctness - Test prediction accuracy on MNIST test set
Usage: python verify_model_correctness.py [exe_path] [num_samples]
Example: python verify_model_correctness.py generated/lenet5_optimized.exe 100
"""
import subprocess
import sys
import os
import re
from pathlib import Path

def main():
    # Parse arguments
    if len(sys.argv) > 1 and not sys.argv[1].isdigit():
        exe_path = sys.argv[1]
        n_test = int(sys.argv[2]) if len(sys.argv) > 2 else 100
    else:
        exe_path = 'bin\\test_lenet5.exe'  # Default
        n_test = int(sys.argv[1]) if len(sys.argv) > 1 else 100
    
    print(f"Testing executable: {exe_path}")
    print(f"Number of samples: {n_test}")
    print()
    
    test_dir = Path('test_data/preprocessed_28x28')
    test_files = sorted(test_dir.glob('mnist_*.bin'))
    
    if not test_files:
        print("ERROR: No test files found in test_data/preprocessed_28x28/")
        return
    
    test_files = test_files[:n_test]
    
    print(f"Running inference on {len(test_files)} images...")
    
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
            [exe_path, str(f)],
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
