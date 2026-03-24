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
- is the sliding window compact enough that a small output-width micro-tile is still locality-friendly?
- is there enough total convolution work for a wider micro-tile to amortize its control cost?

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

## Current Selection Rule

The current planner evaluates legal candidate widths `1`, `2`, and `4`.

For each legal width, it computes a simple reuse-density score:

- `score ~= estimated_conv_work * width / sliding_window_span`

Where:

- `estimated_conv_work` is derived from output size, kernel size, input channels, and padded output channels
- `sliding_window_span` is `((width - 1) * stride) + kernel_width`

Very small layers are still filtered conservatively so the wider choices do not
pay extra control overhead for trivial work.

This keeps the policy general:

- wider micro-tiles win when they buy more adjacent outputs per sliding-window span
- narrower tiles remain available when width, stride, or total work make them the safer choice

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
