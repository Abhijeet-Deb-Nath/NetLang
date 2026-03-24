#!/usr/bin/env python3
"""
Validate the fixed-shape Add/Concat branch examples end-to-end.

This script:
1. Builds the compiler if needed
2. Compiles the AddBranch and ConcatBranch examples to C
3. Builds benchmark executables
4. Runs them on a deterministic synthetic input
5. Verifies dumped outputs against mathematically expected results
"""

from __future__ import annotations

import math
import struct
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
RESULT_DIR = REPO_ROOT / "results" / "evaluation" / "branch_validation"
GENERATED_DIR = REPO_ROOT / "generated"

H = 28
W = 28
C = 4
POOLED_H = 14
POOLED_W = 14


def run(cmd: list[str]) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=REPO_ROOT, check=True)


def write_input(path: Path) -> list[float]:
    values = [float((i % 17) - 8) for i in range(H * W * C)]
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as f:
        for value in values:
            f.write(struct.pack("f", value))
    return values


def load_floats(path: Path) -> list[float]:
    data = path.read_bytes()
    count = len(data) // 4
    return list(struct.unpack("<" + "f" * count, data))


def reshape_hwc(values: list[float], height: int, width: int, channels: int) -> list[list[list[float]]]:
    out = [[[0.0 for _ in range(channels)] for _ in range(width)] for _ in range(height)]
    index = 0
    for h in range(height):
        for w in range(width):
            for c in range(channels):
                out[h][w][c] = values[index]
                index += 1
    return out


def pool_expected(input_hwc: list[list[list[float]]]) -> tuple[list[list[list[float]]], list[list[list[float]]]]:
    left = [[[0.0 for _ in range(C)] for _ in range(POOLED_W)] for _ in range(POOLED_H)]
    right = [[[0.0 for _ in range(C)] for _ in range(POOLED_W)] for _ in range(POOLED_H)]

    for oh in range(POOLED_H):
        for ow in range(POOLED_W):
            for ch in range(C):
                window = [
                    input_hwc[oh * 2 + kh][ow * 2 + kw][ch]
                    for kh in range(2)
                    for kw in range(2)
                ]
                left[oh][ow][ch] = max(window)
                right[oh][ow][ch] = sum(window) / 4.0

    return left, right


def flatten_hwc_to_chw(tensor: list[list[list[float]]]) -> list[float]:
    height = len(tensor)
    width = len(tensor[0])
    channels = len(tensor[0][0])
    out: list[float] = []
    for ch in range(channels):
        for h in range(height):
            for w in range(width):
                out.append(tensor[h][w][ch])
    return out


def expected_add(values: list[float]) -> list[float]:
    input_hwc = reshape_hwc(values, H, W, C)
    left, right = pool_expected(input_hwc)
    merged = [[[left[h][w][ch] + right[h][w][ch] for ch in range(C)] for w in range(POOLED_W)] for h in range(POOLED_H)]
    return flatten_hwc_to_chw(merged)


def expected_concat(values: list[float]) -> list[float]:
    input_hwc = reshape_hwc(values, H, W, C)
    left, right = pool_expected(input_hwc)
    merged = [[[0.0 for _ in range(C * 2)] for _ in range(POOLED_W)] for _ in range(POOLED_H)]
    for h in range(POOLED_H):
        for w in range(POOLED_W):
            for ch in range(C):
                merged[h][w][ch] = left[h][w][ch]
                merged[h][w][C + ch] = right[h][w][ch]
    return flatten_hwc_to_chw(merged)


def max_abs_diff(expected: list[float], actual: list[float]) -> float:
    if len(expected) != len(actual):
        raise ValueError(f"length mismatch: expected {len(expected)}, got {len(actual)}")
    return max(abs(a - b) for a, b in zip(expected, actual)) if expected else 0.0


def validate_example(example_name: str, expected_output: list[float]) -> float:
    c_path = GENERATED_DIR / f"{example_name}_validation.c"
    exe_name = f"{example_name}_validation"
    dump_path = RESULT_DIR / f"{example_name}_output.bin"

    run([str(REPO_ROOT / "build" / "netlang.exe"), f"examples\\{example_name}.nlang", "-o", str(c_path.relative_to(REPO_ROOT))])
    run(["cmd", "/c", "build_benchmark.bat", str(c_path.relative_to(REPO_ROOT)), exe_name])
    run([str(REPO_ROOT / "bin" / f"{exe_name}.exe"),
         "--input", str((RESULT_DIR / "test_input.bin").relative_to(REPO_ROOT)),
         "--warmup", "1",
         "--runs", "1",
         "--dump-output", str(dump_path.relative_to(REPO_ROOT))])

    actual = load_floats(dump_path)
    return max_abs_diff(expected_output, actual)


def main() -> int:
    RESULT_DIR.mkdir(parents=True, exist_ok=True)

    compiler = REPO_ROOT / "build" / "netlang.exe"
    if not compiler.exists():
        run(["cmd", "/c", "build.bat"])

    input_values = write_input(RESULT_DIR / "test_input.bin")
    add_expected = expected_add(input_values)
    concat_expected = expected_concat(input_values)

    add_diff = validate_example("add_branch", add_expected)
    concat_diff = validate_example("concat_branch", concat_expected)

    print(f"add_branch max abs diff: {add_diff}")
    print(f"concat_branch max abs diff: {concat_diff}")

    tolerance = 1e-6
    if add_diff > tolerance or concat_diff > tolerance:
        print("Branch example validation FAILED", file=sys.stderr)
        return 1

    print("Branch example validation PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
