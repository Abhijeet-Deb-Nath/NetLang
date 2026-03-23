# Syntax

This document describes the stable user-facing syntax for the current codegen-ready subset and labels the parser-only features honestly.

## Codegen-Ready Subset

### Network Skeleton

```netlang
network Name {
    input(shape: [H, W, C])
    weights("path/to/model.nwf")  // optional for weightless graphs

    x = Conv2D(filters: 32, kernel: [3, 3], stride: 1, padding: 1, activation: relu) from input
    x = MaxPool(pool: [2, 2], stride: 2, padding: 0) from x
    x = Flatten() from x
    x = Dense(units: 10, activation: softmax) from x
}
```

The `weights("...")` declaration is optional.

Use it when the network contains `Conv2D` or `Dense` layers. Weightless graph examples can omit it entirely.

### Stable Layer Forms

- `Conv2D(filters:, kernel:, stride:, padding:, activation:)`
- `Dense(units:, activation:)`
- `MaxPool(pool:, stride:, padding:)`
- `AvgPool(pool:, stride:, padding:)`
- `Flatten()`
- `Add(x, y, ...)`
- `Concat(x, y, ...)`

### Activations

The parser recognizes:

- `relu`
- `sigmoid`
- `tanh`
- `softmax`
- `linear`

The most reliable current path is `relu` in hidden layers and `softmax` only at the final dense layer.

## Support Matrix

| Feature | Parser | Semantic checks | C code generation |
| --- | --- | --- | --- |
| Sequential assignments | Yes | Yes | Yes |
| `Conv2D` | Yes | Yes | Yes |
| `Dense` | Yes | Yes | Yes |
| `MaxPool` / `AvgPool` | Yes | Yes | Yes |
| `Flatten` | Yes | Yes | Yes |
| `Add` | Yes | Yes | Yes (experimental) |
| `Concat` | Yes | Yes | Yes (experimental) |
| `BatchNorm` | Yes | Partial | No |
| `module` definitions | Yes | Partial | No |

If a feature is not codegen-ready, it must not be documented as a stable end-to-end capability.

## Source Rules

- unary operators such as `Conv2D`, `Dense`, pooling, and `Flatten` require `from source_name`
- multi-input operators `Add(...)` and `Concat(...)` take their inputs directly and do not need `from`
- repeated variable names are allowed and lower to new internal graph values

## Notes For Path 1

The next language-level features that matter are:

- better graph validation
- broader DAG coverage
- front-end sugar only after the primitive graph path is solid
