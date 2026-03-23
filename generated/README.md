# Generated Output

This directory stores C files emitted by the NetLang compiler.

## What Generated Files Represent Today

Generated files currently represent:

- a fixed-shape network
- external `.nwf` weights
- graph-driven scheduling over the currently supported op set
- planner-backed activation arena allocation
- runtime calls into `src/codegen/runtime.c` and `src/codegen/kernels.c`

## What They Do Not Yet Represent

- general DAG coverage beyond the current supported subset
- mature graph-aware optimization beyond the current direct lowering path
- final publication-grade reporting by themselves

## Typical Flow

```powershell
build\netlang.exe examples\lenet5.nlang -o generated\lenet5.c
build_network.bat generated\lenet5.c lenet5_smoke
```

Generated files are compiler artifacts, not hand-maintained source files.
