# Memory Planning

NetLang now has an explicit activation memory planner.

This is the first implementation of the core compiler claim:

- graph-aware activation lifetime analysis
- reusable slots
- static activation arena generation

## Module Layout

The planner lives in:

- `src/planner/memory_plan.h`
- `src/planner/memory_plan.c`

It consumes the graph IR and produces:

- per-value lifetime intervals
- slot assignments
- aligned slot offsets inside a single arena
- total arena size in bytes

## Design Choice

The planner is a separate layer, not part of the graph IR.

That separation is deliberate:

- graph IR represents semantics
- memory planner represents one allocation policy
- codegen consumes the planner result

This keeps the architecture clean enough to swap in a better planner later without rewriting graph construction.

## Current Policy

The current planner is conservative and simple.

For each produced value:

1. `first_step` is the producer node's topological index
2. `last_step` is the last consumer's topological index
3. the final output is kept alive until output copy
4. slots are reused only when the previous value's `last_step` is strictly before the new value's `first_step`

Slot reuse is assigned with a simple best-fit policy:

- reuse the smallest available slot that is large enough
- otherwise create a new slot

## Alignment

Each slot start is aligned to a 64-byte boundary inside the arena.

Because activations are float32 today, this is implemented as alignment to 16 floats.

## Current Output Shape

Generated C now emits:

- one `arena` pointer in `NetworkState`
- one `slot_N` pointer per reusable activation slot
- metadata accessors for arena bytes and slot count

instead of one dedicated allocation per produced tensor value.

## What This Solves

- stops the backend from allocating one long-lived buffer per value
- exposes a concrete compile-time memory metric
- makes that metric available to external runners without exposing planner internals
- gives the repo an actual compiler-analysis step beyond parsing and code emission

## What It Does Not Solve Yet

- no in-place operator analysis
- no alias-aware optimization
- no residual-specific planner optimizations beyond generic slot reuse
- no more advanced global packing policy
- no benchmark harness for memory reporting yet

## Why This Matters

This is the first point where the repo begins to look like a compiler paper path rather than a parser plus C emitter.
