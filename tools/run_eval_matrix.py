#!/usr/bin/env python3
"""
Run a multi-sample evaluation matrix for a NetLang network.

This script compiles the network once, reuses the benchmark executable across
many inputs, and aggregates accuracy, parity, and latency summaries.
"""

from __future__ import annotations

import argparse
import csv
import glob
import json
import re
from pathlib import Path
from typing import Any

import numpy as np

from run_eval import (
    DEFAULT_RESULTS_DIR,
    REPO_ROOT,
    build_netlang_benchmark_artifact,
    compare_outputs,
    ensure_parent,
    format_artifact_path,
    measure_onnxruntime,
    run_netlang_benchmark,
)


LABEL_PATTERN = re.compile(r"_label_(\d+)")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run a multi-sample NetLang evaluation matrix")
    parser.add_argument("--network", required=True, help="Path to the .nlang file")
    parser.add_argument(
        "--input-glob",
        default="assets/inputs/preprocessed_28x28/*.bin",
        help="Glob for preprocessed float32 input samples",
    )
    parser.add_argument("--onnx", help="Optional ONNX model for baseline comparison")
    parser.add_argument("--warmup", type=int, default=5, help="Warmup runs per sample")
    parser.add_argument("--runs", type=int, default=20, help="Measured runs per sample")
    parser.add_argument(
        "--sample-limit",
        type=int,
        default=25,
        help="Maximum number of samples to evaluate; use 0 or negative for all matches",
    )
    parser.add_argument("--prefix", help="Artifact prefix for generated C, binaries, and result files")
    parser.add_argument(
        "--result-dir",
        help="Directory for output summaries (defaults to results/evaluation)",
    )
    parser.add_argument(
        "--onnx-output-transform",
        choices=["auto", "none", "softmax"],
        default="auto",
        help="Transform applied to ONNX Runtime outputs before comparison",
    )
    return parser.parse_args()


def resolve_path(path_str: str) -> Path:
    path = Path(path_str)
    return (REPO_ROOT / path).resolve() if not path.is_absolute() else path.resolve()


def discover_samples(pattern: str, sample_limit: int) -> list[Path]:
    if Path(pattern).is_absolute():
        matches = [Path(p).resolve() for p in glob.glob(pattern)]
    else:
        matches = [Path(p).resolve() for p in glob.glob(str((REPO_ROOT / pattern).as_posix()))]

    matches = sorted(match for match in matches if match.is_file())
    if sample_limit > 0:
        matches = matches[:sample_limit]
    return matches


def extract_label(sample_path: Path) -> int | None:
    match = LABEL_PATTERN.search(sample_path.name)
    if not match:
        return None
    return int(match.group(1))


def compute_summary_metrics(records: list[dict[str, Any]], field: str) -> dict[str, float] | None:
    values = [float(record[field]) for record in records if record.get(field) is not None]
    if not values:
        return None

    array = np.asarray(values, dtype=np.float64)
    return {
        "mean": float(np.mean(array)),
        "median": float(np.median(array)),
        "min": float(np.min(array)),
        "max": float(np.max(array)),
    }


def compute_accuracy(records: list[dict[str, Any]], prediction_key: str) -> dict[str, Any] | None:
    labeled = [record for record in records if record.get("label") is not None and record.get(prediction_key) is not None]
    if not labeled:
        return None

    correct = sum(1 for record in labeled if int(record[prediction_key]) == int(record["label"]))
    return {
        "labeled_samples": len(labeled),
        "correct": correct,
        "accuracy": float(correct / len(labeled)),
    }


def write_csv(path: Path, records: list[dict[str, Any]]) -> None:
    ensure_parent(path)
    fieldnames = [
        "sample",
        "label",
        "netlang_pred",
        "netlang_confidence",
        "netlang_init_ms",
        "netlang_first_ms",
        "netlang_warm_mean_ms",
        "netlang_warm_median_ms",
        "netlang_warm_p95_ms",
        "onnx_pred",
        "onnx_confidence",
        "onnx_init_ms",
        "onnx_first_ms",
        "onnx_warm_mean_ms",
        "onnx_warm_median_ms",
        "onnx_warm_p95_ms",
        "max_abs_diff",
        "mean_abs_diff",
        "rmse",
        "argmax_match",
    ]

    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for record in records:
            writer.writerow({key: record.get(key) for key in fieldnames})


def main() -> None:
    args = parse_args()

    if args.warmup <= 0 or args.runs <= 0:
        raise SystemExit("--warmup and --runs must both be positive")

    network_path = resolve_path(args.network)
    if not network_path.exists():
        raise SystemExit(f"Network file not found: {network_path}")

    samples = discover_samples(args.input_glob, args.sample_limit)
    if not samples:
        raise SystemExit(f"No input samples matched: {args.input_glob}")

    onnx_path = None
    if args.onnx:
        onnx_path = resolve_path(args.onnx)
        if not onnx_path.exists():
            raise SystemExit(f"ONNX file not found: {onnx_path}")

    prefix = args.prefix or f"{network_path.stem}_eval_matrix"
    result_dir = resolve_path(args.result_dir) if args.result_dir else DEFAULT_RESULTS_DIR
    summary_json_path = result_dir / f"{prefix}.json"
    summary_csv_path = result_dir / f"{prefix}.csv"
    temp_dir = result_dir / ".tmp" / prefix

    artifact = build_netlang_benchmark_artifact(network_path, prefix)

    sample_records: list[dict[str, Any]] = []
    benchmark_metadata: dict[str, Any] | None = None
    temp_dir.mkdir(parents=True, exist_ok=True)
    runtime_json_path = temp_dir / "runtime.json"
    output_bin_path = temp_dir / "output.bin"

    for index, sample_path in enumerate(samples):
        netlang_runtime, netlang_output = run_netlang_benchmark(
            artifact["benchmark_exe_path"],
            sample_path,
            args.warmup,
            args.runs,
            runtime_json_path,
            output_bin_path,
        )
        if benchmark_metadata is None:
            benchmark_metadata = netlang_runtime

        record: dict[str, Any] = {
            "sample": format_artifact_path(sample_path),
            "label": extract_label(sample_path),
            "netlang_pred": int(np.argmax(netlang_output)),
            "netlang_confidence": float(np.max(netlang_output)),
            "netlang_init_ms": float(netlang_runtime["init_time_ms"]),
            "netlang_first_ms": float(netlang_runtime["first_inference_ms"]),
            "netlang_warm_mean_ms": float(netlang_runtime["warm_mean_ms"]),
            "netlang_warm_median_ms": float(netlang_runtime["warm_median_ms"]),
            "netlang_warm_p95_ms": float(netlang_runtime["warm_p95_ms"]),
        }

        if onnx_path is not None:
            flat_input = np.fromfile(sample_path, dtype=np.float32)
            ort_results = measure_onnxruntime(
                onnx_path,
                flat_input,
                args.warmup,
                args.runs,
                args.onnx_output_transform,
            )
            ort_output = np.asarray(ort_results.pop("output"), dtype=np.float32)
            comparison = compare_outputs(netlang_output, ort_output)

            record.update(
                {
                    "onnx_pred": int(ort_results["output_argmax"]),
                    "onnx_confidence": float(np.max(ort_output)),
                    "onnx_init_ms": float(ort_results["init_time_ms"]),
                    "onnx_first_ms": float(ort_results["first_inference_ms"]),
                    "onnx_warm_mean_ms": float(ort_results["warm_mean_ms"]),
                    "onnx_warm_median_ms": float(ort_results["warm_median_ms"]),
                    "onnx_warm_p95_ms": float(ort_results["warm_p95_ms"]),
                    "max_abs_diff": comparison.get("max_abs_diff"),
                    "mean_abs_diff": comparison.get("mean_abs_diff"),
                    "rmse": comparison.get("rmse"),
                    "argmax_match": comparison.get("argmax_match"),
                }
            )

        sample_records.append(record)
        print(
            f"[{index + 1}/{len(samples)}] {sample_path.name}: "
            f"netlang={record['netlang_pred']}"
            + (f", onnx={record['onnx_pred']}" if "onnx_pred" in record else "")
        )

    summary: dict[str, Any] = {
        "network": network_path.name,
        "input_glob": args.input_glob,
        "sample_count": len(sample_records),
        "artifacts": {
            "generated_c": format_artifact_path(artifact["generated_c_path"]),
            "benchmark_exe": format_artifact_path(artifact["benchmark_exe_path"]),
            "summary_json": format_artifact_path(summary_json_path),
            "summary_csv": format_artifact_path(summary_csv_path),
        },
        "netlang": {
            "codegen_ms": artifact["codegen_ms"],
            "native_build_ms": artifact["native_build_ms"],
            "total_build_ms": artifact["total_build_ms"],
            "accuracy": compute_accuracy(sample_records, "netlang_pred"),
            "init_ms": compute_summary_metrics(sample_records, "netlang_init_ms"),
            "first_inference_ms": compute_summary_metrics(sample_records, "netlang_first_ms"),
            "warm_mean_ms": compute_summary_metrics(sample_records, "netlang_warm_mean_ms"),
            "warm_median_ms": compute_summary_metrics(sample_records, "netlang_warm_median_ms"),
            "warm_p95_ms": compute_summary_metrics(sample_records, "netlang_warm_p95_ms"),
        },
        "samples": sample_records,
    }

    if benchmark_metadata is not None:
        summary["netlang"]["activation_arena_bytes"] = int(benchmark_metadata["activation_arena_bytes"])
        summary["netlang"]["activation_slot_count"] = int(benchmark_metadata["activation_slot_count"])
        summary["netlang"]["executable_size_bytes"] = int(benchmark_metadata["executable_size_bytes"])

    if onnx_path is not None:
        summary["onnxruntime"] = {
            "onnx_model": str(onnx_path),
            "accuracy": compute_accuracy(sample_records, "onnx_pred"),
            "init_ms": compute_summary_metrics(sample_records, "onnx_init_ms"),
            "first_inference_ms": compute_summary_metrics(sample_records, "onnx_first_ms"),
            "warm_mean_ms": compute_summary_metrics(sample_records, "onnx_warm_mean_ms"),
            "warm_median_ms": compute_summary_metrics(sample_records, "onnx_warm_median_ms"),
            "warm_p95_ms": compute_summary_metrics(sample_records, "onnx_warm_p95_ms"),
        }
        summary["comparison"] = {
            "max_abs_diff": compute_summary_metrics(sample_records, "max_abs_diff"),
            "mean_abs_diff": compute_summary_metrics(sample_records, "mean_abs_diff"),
            "rmse": compute_summary_metrics(sample_records, "rmse"),
            "argmax_match_rate": (
                float(np.mean([1.0 if record.get("argmax_match") else 0.0 for record in sample_records]))
                if sample_records
                else None
            ),
        }

    ensure_parent(summary_json_path)
    with summary_json_path.open("w", encoding="utf-8") as handle:
        json.dump(summary, handle, indent=2)

    write_csv(summary_csv_path, sample_records)

    print("Matrix evaluation complete")
    print(f"Summary JSON: {summary_json_path}")
    print(f"Summary CSV: {summary_csv_path}")
    if summary["netlang"]["accuracy"] is not None:
        print(f"NetLang accuracy: {summary['netlang']['accuracy']['accuracy']:.3f}")
    if "onnxruntime" in summary and summary["onnxruntime"]["accuracy"] is not None:
        print(f"ONNX Runtime accuracy: {summary['onnxruntime']['accuracy']['accuracy']:.3f}")


if __name__ == "__main__":
    main()
