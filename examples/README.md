# Examples

This directory contains the user-facing example networks.

## Current Examples

| File | Purpose | Status |
| --- | --- | --- |
| `simple_classifier.nlang` | Minimal syntax example | Stable |
| `lenet5.nlang` | Reference sequential CNN for smoke testing | Stable |
| `add_branch.nlang` | Minimal branch-and-merge graph example with `Add` | Stable |
| `concat_branch.nlang` | Minimal branch-and-merge graph example with `Concat` | Stable |
| `vgg16.nlang` | Large sequential syntax stress example | Exploratory |

## Guidance

- use `lenet5.nlang` as the reference end-to-end example today
- use `add_branch.nlang` and `concat_branch.nlang` when testing the stable fixed-shape merge operators
- treat `vgg16.nlang` as a language example, not as the primary benchmark target
- keep new examples inside the codegen-ready subset unless they are explicitly labeled experimental

## Public vs Internal Inputs

- `examples/` is for human-facing sample networks
- `fixtures/` contains parser fixtures and legacy experiment inputs

## Compile an Example

```powershell
build\netlang.exe examples\lenet5.nlang -o generated\lenet5.c
build_network.bat generated\lenet5.c lenet5_smoke
```

## Syntax Reference

See [Syntax](../docs/SYNTAX.md).
