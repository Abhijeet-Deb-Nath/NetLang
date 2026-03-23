# Architecture

This document describes the current compiler structure and the current research target.

## Current Pipeline

Today the compiler is organized as:

1. lexer
2. parser
3. AST construction
4. semantic checks and shape inference
5. graph IR lowering
6. memory planning
7. direct C code generation from graph order and slot plan

Relevant directories:

- `src/lexer`
- `src/parser`
- `src/ast`
- `src/semantic`
- `src/codegen`

## Current Technical Shape

The repo is no longer pretending to be broader than it is.

Current end-to-end support is strongest for sequential CNNs with:

- `Conv2D`
- `Dense`
- `MaxPool`
- `AvgPool`
- `Flatten`

The major architectural limitations are now:

- graph support is still partial rather than complete
- merge coverage is still limited to primitive `Add` and `Concat`
- the current optimization pass is shallow and not graph-aware
- planner reporting is still basic and not yet tied to the final benchmark harness

## Target Architecture

The target system is a static DAG compiler for fixed-shape CNN inference.

The intended pipeline is:

1. Parse `.nlang` into AST
2. Lower AST into a graph IR
3. Validate graph structure and tensor compatibility
4. Topologically schedule operations
5. Run liveness analysis on activation values
6. Build a static memory plan or arena assignment
7. Emit C code against that schedule and memory plan

## Planned Internal Split

### Frontend

Keep:

- `src/lexer`
- `src/parser`
- `src/ast`

The frontend should stay simple. This is not a frontend-language paper.

### Graph IR

The graph layer now lives under:

- `src/graph/graph.h`
- `src/graph/graph.c`

It explicitly models:

- values
- ops
- producer and consumer links
- multiple consumers
- network inputs and outputs

### Semantic and Lowering Layer

Refactor semantic work so it produces graph-aware information instead of only annotating the AST.

Primary files to revisit:

- `src/semantic/semantic.c`
- `src/semantic/type_checker.c`

### Scheduling and Memory Planning

This is the real compiler contribution for the current research target.

Topological scheduling now exists through the graph layer.

The planner layer now computes:

- value last-use tracking
- reusable activation slots
- aligned offsets inside a single arena

What is still missing:

- richer policies than the current conservative best-fit reuse
- emitted reporting of peak memory statistics
- integration with future residual and broader DAG support

### Code Generation

Refactor `src/codegen/codegen.c` so it consumes graph IR plus memory-plan results rather than implicit previous-layer chaining.

## Immediate Refactor Priorities

1. Tighten the planner and expose richer memory metrics in generated artifacts
2. Add ablation-friendly allocation modes so planner impact can be measured directly
3. Broaden evaluation around graph-aware workloads and additional trusted models
4. Extend the supported op set only when it strengthens the fixed-shape DAG thesis

## What This Repo Is Not Doing

The current research target does not require:

- dynamic shapes
- training
- GPU backends
- model-zoo syntax as the main research contribution
- broad framework parity

If a change does not help graph correctness, memory planning, or evaluation quality, it is secondary.
