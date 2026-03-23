# Graph IR

NetLang now lowers networks into an explicit graph IR before code generation.

This is the first real compiler milestone for the current research target.

## Why It Exists

The previous backend mostly treated the program like a linear chain:

- layer `i` consumes layer `i - 1`
- source order was execution order
- buffer naming and dependency handling were implicit

That was not enough for the current research target.

The graph IR makes dependencies explicit.

## Core Model

The implementation lives in:

- `src/graph/graph.h`
- `src/graph/graph.c`

It uses two core objects:

- `GraphValue`: a tensor value with a producer, consumers, shape, and generated storage name
- `GraphNode`: an operator with typed input values and one output value

The whole network is stored as `NetGraph`.

## Important Design Choice

The graph builder treats repeated source names as new values.

Example:

```netlang
x = Conv2D(...) from input
x = MaxPool(...) from x
```

This becomes two different internal values.

That makes the graph closer to SSA form and prevents the backend from confusing source-level variable reuse with one physical tensor.

## Lowering Rules

Current graph lowering supports:

- `Conv2D`
- `Dense`
- `MaxPool`
- `AvgPool`
- `Flatten`
- `Add`
- `Concat`

The builder:

1. parses input shape from the network header
2. creates an input value
3. walks assignments in source order
4. resolves each source name to the latest current value
5. computes output tensor shape
6. creates producer and consumer edges
7. runs topological scheduling

## Topological Scheduling

Even though today’s language still encourages source-order definitions, the backend no longer relies on "previous layer" assumptions.

The graph module computes a topological order from value dependencies.

That is the order code generation now uses.

## Current Limitations

- no module-call lowering
- planner-driven reporting is not yet exposed to users
- graph lowering is still fixed-shape only

## Why This Matters

This change is the foundation for the next two compiler steps:

- activation liveness analysis
- static buffer reuse / arena planning

Without explicit graph values and producer-consumer edges, those steps would stay ad hoc.
