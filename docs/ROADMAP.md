# Roadmap

This is the governing roadmap for the repo.

## Working Thesis

NetLang should become:

> a static compiler for fixed-shape CNN DAGs that provides compile-time activation memory planning, low-overhead AOT deployment, and honest CPU-side evaluation

That is the paper direction. Everything else is secondary.

## Research Claim

The intended claim is not:

- "a CNN DSL is faster because Python is interpreted"

The intended claim is:

- a restricted compiler stack can trade generality for predictability
- fixed-shape CNN graphs allow compile-time buffer planning
- external late-bound weights can keep deployment flexible without forcing a heavyweight runtime

## Scope

In scope:

- inference only
- fixed shapes
- static DAGs
- batch size 1 as the main evaluation mode
- CPU target first
- `Conv2D`, `Dense`, pooling, `Flatten`, `Add`, `Concat`

Out of scope for the core paper:

- training
- dynamic shapes
- GPU backends
- automatic model import from every framework as the main story
- syntax sugar as the main contribution

## Milestones

### M0. Scope Lock

Done in this repo reorganization:

- remove misleading docs
- narrow public claims
- define the support matrix and benchmark protocol

### M1. Graph IR

Deliverables:

- explicit internal graph representation
- values with producer and consumer tracking
- lower existing sequential examples through the graph IR

Status:

- implemented
- code generation now consumes topologically scheduled graph nodes instead of implicit previous-layer chaining

Primary files:

- new `src/graph/*`
- `src/semantic/semantic.c`

### M2. DAG Semantics

Deliverables:

- `Add`
- `Concat`
- topological scheduling
- graph validation for fan-out and fan-in

Status:

- topological scheduling is implemented
- `Add` and `Concat` are implemented as the first generated multi-input operators

### M3. Memory Planning

Deliverables:

- activation lifetime analysis
- static buffer reuse
- peak activation memory report
- emitted code that uses a plan instead of one buffer per layer

Status:

- initial version implemented
- generated code now uses a static arena plus reusable slots
- generated artifacts now expose basic arena-size and slot-count metadata
- peak-memory reporting is not yet integrated into the final evaluation harness

This remains the core compiler value and still needs strengthening.

### M4. Evaluation Harness

Deliverables:

- compile-time measurement
- init and load-time measurement
- first inference latency
- warm inference latency
- peak memory
- binary size

The current smoke harness is not enough.

Status:

- in-process benchmark harness implemented
- generated artifacts expose enough metadata for generic runtime measurements
- compile-time measurement and ONNX Runtime comparison are now automated for the reference path
- multi-sample reference-path aggregation is now automated
- broader multi-model result collection still remains

### M5. Paper Packaging

Deliverables:

- ablation study
- correctness checks against a trusted baseline
- small but honest model suite
- writeup centered on the actual stable backend and its measured limits

## Current Next Step

The repo is no longer on the region-tiling branch.

The current stable base is:

- graph lowering
- static activation arena planning
- packed Conv2D weights
- output-width Conv2D micro-tiling
- kernel selection from operator/layout facts
- persistent runtime threadpool

Any next optimization direction should start from that base and keep the public
message coherent around the measured CPU inference path.

## Evaluation Targets

Primary outcome measures:

- peak activation memory
- cold start and first inference latency
- warm batch-1 latency
- generated artifact size

Secondary outcome measures:

- compile time
- weight-load overhead
- correctness parity with a baseline runtime

## Candidate Publication Path

Best fit from this repo state:

- conference-first: compiler, embedded, or ML-systems venues
- long-form target after that: TACO-style compiler or systems journal path

This is not a good repo for a generic "new deep learning architecture language" paper.
