# Status

This file is the honest support matrix for the repo.

## Compiler Status

| Area | Current state | Research target |
| --- | --- | --- |
| Frontend | Working lexer, parser, and AST for `.nlang` | Keep stable |
| Semantic checks | Strongest on sequential CNNs | Graph-aware validation |
| Internal representation | Explicit graph IR for supported ops | Keep and extend |
| Scheduling | Topological scheduling over graph values | Broaden beyond current op set |
| Activation memory | Static arena with slot reuse | Better planner and explicit reporting |
| Execution backend | Shape/layout-driven kernel selection plus runtime threadpool | Broader execution planning and cross-op scheduling |
| Codegen | Graph-driven codegen with packed Conv2D and layout-specialized Dense paths | Static DAG codegen with richer graph-aware optimization |
| Kernel policy | Explicit backend kernel selection layer | Better cost-model-driven selection |
| Weight loading | External `.nwf` runtime loading | Late-bound packed weights |
| Benchmarking | Smoke-test level | Publication-grade protocol |

## Stable End-to-End Subset

- fixed-shape sequential CNNs
- `Conv2D`
- `Dense`
- `MaxPool`
- `AvgPool`
- `Flatten`

## Experimental Graph Support

- branch fan-out from shared source values
- `Add` merge
- `Concat` merge
- graph-driven storage naming
- topological scheduling
- slot-based activation reuse
- alias-aware `Flatten -> Dense` specialization with repacked dense weights
- OC8-packed Conv2D backend for AVX2 output-channel vectorization
- output-width micro-tiling plan for packed Conv2D execution
- runtime threadpool with shape-driven packed Conv2D parallel execution

## Parser-Oriented But Not Yet Honest To Claim

- broader residual-network coverage
- reusable modules as a real lowering feature
- broad converter parity

## Immediate Blockers

1. multi-model and broader baseline result collection are not automated yet
2. planner reporting is basic and not yet a full peak-memory evaluation path
3. graph support is still limited to the currently generated primitive op set
4. execution planning is still per-op rather than cross-op
5. current backend is still materially slower than ONNX Runtime on the reference path

## Current Reference Result

On the current 25-sample LeNet reference matrix:

- NetLang warm-mean average: `1.002 ms` in `results/evaluation/lenet5_stable_matrix_25.json`
- ONNX Runtime warm-mean average: `0.240 ms` on the same matrix
- accuracy: `25/25`
- argmax match rate: `1.0`

That places the current stable mainline at roughly `4.2x` slower than the
local ONNX Runtime CPU baseline on the reference workload.

## Repository Policy

- keep docs aligned with the generated-code reality
- do not present parser-only features as shipped runtime support
- prefer fewer honest claims over a broader but misleading feature list
