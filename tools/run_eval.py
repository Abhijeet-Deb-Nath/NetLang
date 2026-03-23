#!/usr/bin/env python3
"""
Evaluation driver for NetLang.

This script orchestrates:
1. NetLang code generation
2. Native benchmark executable build
3. NetLang runtime measurement
4. Optional ONNX Runtime baseline measurement
5. Output comparison and result export
"""

from __future__ import annotations

import argparse
import json
import math
import subprocess
import sys
import time
from pathlib import Path
from typing import Any

import numpy as np

try:
    import onnxruntime as ort
except ImportError:  # pragma: no cover - handled at runtime
    ort = None


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_RESULTS_DIR = REPO_ROOT / "results" / "evaluation"


def run_command(command: list[str], cwd: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command,
        cwd=str(cwd),
        text=True,
        capture_output=True,
        check=False,
    )


def timed_command(command: list[str], cwd: Path) -> tuple[float, subprocess.CompletedProcess[str]]:
    start = time.perf_counter()
    completed = run_command(command, cwd)
    elapsed_ms = (time.perf_counter() - start) * 1000.0
    return elapsed_ms, completed


def require_success(step_name: str, completed: subprocess.CompletedProcess[str]) -> None:
    if completed.returncode == 0:
        return

    sys.stderr.write(f"\n[{step_name}] failed with exit code {completed.returncode}\n")
    if completed.stdout:
        sys.stderr.write("\n--- stdout ---\n")
        sys.stderr.write(completed.stdout)
    if completed.stderr:
        sys.stderr.write("\n--- stderr ---\n")
        sys.stderr.write(completed.stderr)
    raise SystemExit(1)


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def format_artifact_path(path: Path) -> str:
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def calculate_stats(samples_ms: np.ndarray) -> dict[str, float]:
    sorted_samples = np.sort(samples_ms.astype(np.float64))
    return {
        "warm_mean_ms": float(np.mean(sorted_samples)),
        "warm_median_ms": float(np.median(sorted_samples)),
        "warm_p95_ms": float(np.percentile(sorted_samples, 95)),
        "warm_min_ms": float(sorted_samples[0]),
        "warm_max_ms": float(sorted_samples[-1]),
        "warm_stddev_ms": float(np.std(sorted_samples)),
        "warm_throughput_ips": float(1000.0 / np.mean(sorted_samples)) if np.mean(sorted_samples) > 0 else 0.0,
    }


def resolve_onnx_dims(shape: list[Any], flat_input_size: int) -> list[int]:
    resolved: list[int] = []
    unknown_indices: list[int] = []

    for index, dim in enumerate(shape):
        if isinstance(dim, int) and dim > 0:
            resolved.append(dim)
        else:
            resolved.append(-1)
            unknown_indices.append(index)

    if not unknown_indices:
        return resolved

    if len(unknown_indices) == 1:
        known_product = 1
        for dim in resolved:
            if dim > 0:
                known_product *= dim
        if known_product <= 0 or flat_input_size % known_product != 0:
            raise ValueError(f"Cannot resolve ONNX input shape {shape} from {flat_input_size} elements")
        resolved[unknown_indices[0]] = flat_input_size // known_product
        return resolved

    raise ValueError(f"Unsupported ONNX input shape with multiple unknown dims: {shape}")


def prepare_onnx_input(flat_input: np.ndarray, input_shape: list[Any]) -> np.ndarray:
    resolved_shape = resolve_onnx_dims(input_shape, int(flat_input.size))

    if len(resolved_shape) == 4:
        batch, channels, height, width = resolved_shape
        if batch != 1:
            raise ValueError(f"Only batch=1 is supported today, got ONNX input shape {resolved_shape}")
        expected_elements = channels * height * width
        if expected_elements != flat_input.size:
            raise ValueError(
                f"Input size mismatch: ONNX expects {expected_elements} elements, file has {flat_input.size}"
            )
        hwc = flat_input.reshape(height, width, channels)
        nchw = np.transpose(hwc, (2, 0, 1))
        return nchw.reshape(1, channels, height, width).astype(np.float32)

    if len(resolved_shape) == 2:
        batch, features = resolved_shape
        if batch != 1:
            raise ValueError(f"Only batch=1 is supported today, got ONNX input shape {resolved_shape}")
        if features != flat_input.size:
            raise ValueError(
                f"Input size mismatch: ONNX expects {features} elements, file has {flat_input.size}"
            )
        return flat_input.reshape(1, features).astype(np.float32)

    if len(resolved_shape) == 1:
        if resolved_shape[0] != flat_input.size:
            raise ValueError(
                f"Input size mismatch: ONNX expects {resolved_shape[0]} elements, file has {flat_input.size}"
            )
        return flat_input.astype(np.float32)

    raise ValueError(f"Unsupported ONNX input rank: {len(resolved_shape)}")


def apply_output_transform(output: np.ndarray, transform: str) -> tuple[np.ndarray, str]:
    if transform == "none":
        return output.astype(np.float32), "none"

    if transform == "softmax":
        shifted = output.astype(np.float64) - np.max(output)
        exp_output = np.exp(shifted)
        return (exp_output / np.sum(exp_output)).astype(np.float32), "softmax"

    if transform != "auto":
        raise ValueError(f"Unsupported output transform mode: {transform}")

    if np.any(output < 0.0) or not np.isclose(np.sum(output), 1.0, atol=1e-3):
        shifted = output.astype(np.float64) - np.max(output)
        exp_output = np.exp(shifted)
        return (exp_output / np.sum(exp_output)).astype(np.float32), "softmax(auto)"

    return output.astype(np.float32), "none(auto)"


def measure_onnxruntime(
    onnx_path: Path,
    flat_input: np.ndarray,
    warmup_runs: int,
    bench_runs: int,
    output_transform: str,
) -> dict[str, Any]:
    if ort is None:
        raise RuntimeError("onnxruntime is not installed")

    init_start = time.perf_counter()
    session = ort.InferenceSession(str(onnx_path))
    init_time_ms = (time.perf_counter() - init_start) * 1000.0

    input_info = session.get_inputs()[0]
    output_info = session.get_outputs()[0]
    prepared_input = prepare_onnx_input(flat_input, input_info.shape)
    feeds = {input_info.name: prepared_input}

    first_start = time.perf_counter()
    first_outputs = session.run(None, feeds)
    first_inference_ms = (time.perf_counter() - first_start) * 1000.0

    for _ in range(warmup_runs):
        session.run(None, feeds)

    samples = np.zeros(bench_runs, dtype=np.float64)
    for index in range(bench_runs):
        start = time.perf_counter()
        outputs = session.run(None, feeds)
        samples[index] = (time.perf_counter() - start) * 1000.0

    raw_output = np.asarray(outputs[0], dtype=np.float32).reshape(-1)
    final_output, applied_transform = apply_output_transform(raw_output, output_transform)
    stats = calculate_stats(samples)
    return {
        "backend": "onnxruntime",
        "onnx_model": str(onnx_path),
        "session_input_name": input_info.name,
        "session_output_name": output_info.name,
        "output_transform": applied_transform,
        "init_time_ms": init_time_ms,
        "first_inference_ms": first_inference_ms,
        "raw_output_checksum": float(np.sum(raw_output, dtype=np.float64)),
        "output_checksum": float(np.sum(final_output, dtype=np.float64)),
        "output_argmax": int(np.argmax(final_output)),
        "output": final_output.tolist(),
        **stats,
    }


def compare_outputs(netlang_output: np.ndarray, ort_output: np.ndarray) -> dict[str, Any]:
    if netlang_output.shape != ort_output.shape:
        return {
            "shape_match": False,
            "netlang_elements": int(netlang_output.size),
            "onnxruntime_elements": int(ort_output.size),
        }

    diff = netlang_output.astype(np.float64) - ort_output.astype(np.float64)
    abs_diff = np.abs(diff)
    return {
        "shape_match": True,
        "max_abs_diff": float(np.max(abs_diff)),
        "mean_abs_diff": float(np.mean(abs_diff)),
        "rmse": float(math.sqrt(np.mean(diff * diff))),
        "argmax_match": bool(int(np.argmax(netlang_output)) == int(np.argmax(ort_output))),
    }


def build_netlang_benchmark_artifact(network_path: Path, prefix: str) -> dict[str, Any]:
    generated_c_path = REPO_ROOT / "generated" / f"{prefix}.c"
    benchmark_exe_path = REPO_ROOT / "bin" / f"{prefix}.exe"

    ensure_parent(generated_c_path)

    codegen_ms, codegen = timed_command(
        ["build\\netlang.exe", str(network_path.relative_to(REPO_ROOT)), "-o", str(generated_c_path.relative_to(REPO_ROOT))],
        REPO_ROOT,
    )
    require_success("netlang-codegen", codegen)

    native_build_ms, native_build = timed_command(
        ["cmd", "/c", "build_benchmark.bat", str(generated_c_path.relative_to(REPO_ROOT)), prefix],
        REPO_ROOT,
    )
    require_success("native-build", native_build)

    return {
        "generated_c_path": generated_c_path,
        "benchmark_exe_path": benchmark_exe_path,
        "codegen_ms": codegen_ms,
        "native_build_ms": native_build_ms,
        "total_build_ms": codegen_ms + native_build_ms,
    }


def run_netlang_benchmark(
    benchmark_exe_path: Path,
    input_path: Path | None,
    warmup_runs: int,
    bench_runs: int,
    runtime_json_path: Path,
    output_bin_path: Path,
) -> tuple[dict[str, Any], np.ndarray]:
    benchmark_command = [
        str(benchmark_exe_path.relative_to(REPO_ROOT)),
        "--warmup",
        str(warmup_runs),
        "--runs",
        str(bench_runs),
        "--json-out",
        str(runtime_json_path),
        "--dump-output",
        str(output_bin_path),
    ]
    if input_path:
        benchmark_command.extend(["--input", str(input_path.relative_to(REPO_ROOT))])

    ensure_parent(runtime_json_path)
    ensure_parent(output_bin_path)

    benchmark_run = run_command(benchmark_command, REPO_ROOT)
    require_success("netlang-benchmark", benchmark_run)

    with runtime_json_path.open("r", encoding="utf-8") as handle:
        runtime_json = json.load(handle)

    output = np.fromfile(output_bin_path, dtype=np.float32)
    return runtime_json, output


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run a NetLang evaluation")
    parser.add_argument("--network", required=True, help="Path to the .nlang file")
    parser.add_argument("--input", help="Path to a preprocessed float32 .bin input")
    parser.add_argument("--onnx", help="Optional ONNX model for baseline comparison")
    parser.add_argument("--warmup", type=int, default=20, help="Warmup runs for runtime measurement")
    parser.add_argument("--runs", type=int, default=200, help="Measured runs for runtime measurement")
    parser.add_argument("--prefix", help="Artifact prefix for generated C and binaries")
    parser.add_argument("--result-json", help="Path to the combined result JSON")
    parser.add_argument(
        "--onnx-output-transform",
        choices=["auto", "none", "softmax"],
        default="auto",
        help="Transform applied to ONNX Runtime outputs before comparison",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    if args.warmup <= 0 or args.runs <= 0:
        raise SystemExit("--warmup and --runs must both be positive")

    network_path = (REPO_ROOT / args.network).resolve() if not Path(args.network).is_absolute() else Path(args.network)
    if not network_path.exists():
        raise SystemExit(f"Network file not found: {network_path}")

    input_path = None
    flat_input = None
    if args.input:
        input_path = (REPO_ROOT / args.input).resolve() if not Path(args.input).is_absolute() else Path(args.input)
        if not input_path.exists():
            raise SystemExit(f"Input file not found: {input_path}")
        flat_input = np.fromfile(input_path, dtype=np.float32)

    onnx_path = None
    if args.onnx:
        onnx_path = (REPO_ROOT / args.onnx).resolve() if not Path(args.onnx).is_absolute() else Path(args.onnx)
        if not onnx_path.exists():
            raise SystemExit(f"ONNX file not found: {onnx_path}")
        if flat_input is None:
            raise SystemExit("--onnx requires --input so outputs can be compared on the same sample")

    prefix = args.prefix or f"{network_path.stem}_eval"
    result_json_path = (
        Path(args.result_json).resolve()
        if args.result_json
        else DEFAULT_RESULTS_DIR / f"{prefix}.json"
    )
    benchmark_json_path = result_json_path.with_name(f"{prefix}_netlang_runtime.json")
    benchmark_output_path = result_json_path.with_name(f"{prefix}_netlang_output.bin")

    ensure_parent(benchmark_json_path)
    ensure_parent(result_json_path)

    artifact = build_netlang_benchmark_artifact(network_path, prefix)
    netlang_runtime, netlang_output = run_netlang_benchmark(
        artifact["benchmark_exe_path"],
        input_path,
        args.warmup,
        args.runs,
        benchmark_json_path,
        benchmark_output_path,
    )

    results: dict[str, Any] = {
        "network": network_path.name,
        "artifacts": {
            "generated_c": format_artifact_path(artifact["generated_c_path"]),
            "benchmark_exe": format_artifact_path(artifact["benchmark_exe_path"]),
            "netlang_runtime_json": format_artifact_path(benchmark_json_path),
            "netlang_output_bin": format_artifact_path(benchmark_output_path),
        },
        "netlang": {
            "codegen_ms": artifact["codegen_ms"],
            "native_build_ms": artifact["native_build_ms"],
            "total_build_ms": artifact["total_build_ms"],
            "runtime": netlang_runtime,
        },
    }

    if onnx_path and flat_input is not None:
        ort_results = measure_onnxruntime(
            onnx_path,
            flat_input,
            args.warmup,
            args.runs,
            args.onnx_output_transform,
        )
        ort_output = np.asarray(ort_results.pop("output"), dtype=np.float32)
        results["onnxruntime"] = ort_results
        results["comparison"] = compare_outputs(netlang_output, ort_output)

    ensure_parent(result_json_path)
    with result_json_path.open("w", encoding="utf-8") as handle:
        json.dump(results, handle, indent=2)

    print("Evaluation complete")
    print(f"Result JSON: {result_json_path}")
    print(f"NetLang build: {artifact['total_build_ms']:.3f} ms")
    print(f"NetLang warm mean: {netlang_runtime['warm_mean_ms']:.3f} ms")
    if "onnxruntime" in results:
        print(f"ONNX Runtime warm mean: {results['onnxruntime']['warm_mean_ms']:.3f} ms")
        if "comparison" in results and results["comparison"].get("shape_match"):
            print(f"Max abs diff: {results['comparison']['max_abs_diff']:.6f}")


if __name__ == "__main__":
    main()
