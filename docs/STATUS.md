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
| Codegen | Graph-driven codegen for supported ops | Static DAG codegen with memory planning |
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

## Parser-Oriented But Not Yet Honest To Claim

- broader residual-network coverage
- reusable modules as a real lowering feature
- broad converter parity

## Immediate Blockers

1. multi-model and broader baseline result collection are not automated yet
2. planner reporting is basic and not yet a full peak-memory evaluation path
3. graph support is still limited to the currently generated primitive op set

## Repository Policy

- keep docs aligned with the generated-code reality
- do not present parser-only features as shipped runtime support
- prefer fewer honest claims over a broader but misleading feature list
