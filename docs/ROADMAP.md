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
- writeup centered on graph scheduling and memory planning

## Near-Term Research Choices

After the current evaluation plumbing, the repo has three reasonable next branches.

### Choice 1. Ablation Study

What it means:

- isolate the effect of graph lowering
- isolate the effect of static arena planning
- isolate the effect of merge-capable code generation

Why it matters:

- this is the strongest evidence that the compiler work changes behavior for a reason
- it turns the paper from "we built a system" into "these compiler choices caused these results"

What it likely requires:

- a switch to emit conservative non-reuse storage for comparison
- planner-on versus planner-off measurements
- graph-aware example coverage for `Add` and `Concat`

Risk:

- medium engineering cost
- highest paper value

### Choice 2. Stronger Memory Reporting

What it means:

- expose planner metrics more clearly
- report planned arena bytes alongside executable size and runtime data
- add optional planner traces or per-value lifetime dumps for analysis

Why it matters:

- memory planning is the cleanest compiler claim in this repo
- stronger memory evidence fits the research direction better than broad syntax work

What it likely requires:

- richer planner metadata export
- benchmark JSON support for memory-plan details
- optional offline report generation from the planner

Risk:

- lower engineering risk than ablation
- narrower evidence value on its own

### Choice 3. Multi-Model Evaluation

What it means:

- add at least one more trustworthy model path beyond the current LeNet reference
- keep ONNX-parity checking for every evaluated model
- broaden the evaluation from one known-correct path to a small honest suite

Why it matters:

- a paper cannot rely forever on one reference network
- broader model coverage makes the evaluation more credible

What it likely requires:

- another verified converter path or hand-audited weight source
- additional preprocessed inputs
- more benchmark automation and result curation

Risk:

- highest evaluation benefit after correctness is trustworthy
- also the easiest place to introduce weak evidence if the extra models are not audited carefully

## Recommended Order

1. ablation study
2. stronger memory reporting
3. multi-model evaluation

That order keeps the next work causal and honest. It is better to explain one model deeply than to rush into a broader but shaky model suite.

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
