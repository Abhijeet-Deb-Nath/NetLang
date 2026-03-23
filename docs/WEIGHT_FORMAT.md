# Weight Format

NetLang uses `.nwf` files to keep weights separate from architecture source.

For the current research target, that split is deliberate:

- the DSL stays focused on graph structure
- weights can be compiled, repacked, or swapped later
- the runtime can load aligned binary tensors without carrying a framework runtime

## Current Format Reality

The current runtime header in `src/codegen/runtime.h` expects:

- magic: `NWGT`
- versioned binary header
- per-layer metadata table
- aligned weight and bias payloads

The public documentation now follows that implementation instead of older inconsistent writeups.

## Current Intent

The format is used for:

- external weight storage
- simple runtime validation
- direct pointer-based access inside the runtime

The format is not yet the final research contribution.

The research value appears when it is paired with:

- graph-aware code generation
- late-bound weight packing
- deployment-oriented evaluation

## Converter Status

Current practical stance:

- ONNX conversion is the strongest path to trust first
- PyTorch and Keras converters should be treated as experimental until re-audited

Do not make publication claims that depend on every converter being equally reliable.

## Data Layout Notes

Current runtime assumptions:

- conv weights are consumed in an output-channel-major kernel layout
- dense weights are stored as layer-local binary tensors and read directly by the generated runtime
- alignment matters for predictable CPU-side access, but alignment alone is not the paper claim

## Direction

The weight format should evolve toward:

- reliable validation against the compiled graph
- optional offline repacking for target kernels
- explicit support for late-bound deployment

That is enough for the current repo direction. The file format does not need to become the main story.
