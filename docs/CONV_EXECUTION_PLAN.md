# Conv Execution Plan

This document describes the current generalized execution planning used for
packed `Conv2D` layers.

## Goal

The current conv planner exists to improve primitive execution without falling
into architecture-specific dispatch.

It does not ask:

- is this LeNet?
- is this ResNet?
- is this a benchmark-special case?

It asks:

- does this `Conv2D` use packed OC8 weights?
- does the output have enough width to reuse packed weights across adjacent outputs?
- is the layer large enough for that reuse to amortize extra control overhead?
- is the sliding window compact enough that a small output-width micro-tile is still locality-friendly?

## Current Plan Surface

For each emitted `Conv2D`, the planner currently chooses:

- `spatial_block_width = 1`
- `spatial_block_width = 2`
- `spatial_block_width = 4`

That width means: compute that many adjacent output positions together while
reusing the same loaded packed weight vector across them.

## What The Planner Uses

The current planner uses only general operator/runtime facts:

- packed-weight availability
- output width
- stride
- kernel width
- input channels
- padded output channels
- estimated total convolution work

This is still heuristic, but it is heuristic over execution properties, not
architecture identities.

## Why This Is General

This plan is applicable to any supported fixed-shape standard `Conv2D` layer
that lowers into the packed OC8 backend.

It is not tied to:

- a named CNN family
- a specific network file
- a fixed layer count

If a custom CNN contains supported `Conv2D` layers with shapes that satisfy the
planner conditions, it uses the same optimization automatically.

## Current Limitation

This is still a **per-op** execution strategy.

It improves local spatial reuse inside a packed `Conv2D`, but it does not yet:

- coordinate multiple ops together
- propagate a global traffic objective through the graph
- autotune plan choices offline

Those remain future directions.
