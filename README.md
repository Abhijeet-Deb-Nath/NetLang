# NetLang Compiler

A domain-specific language compiler for neural network inference, targeting native machine code via LLVM.

## Pipeline

```
NetLang Source (.nlang)
        ↓
   [1] Lexer (Flex)           ← ✅ Complete
        ↓
   [2] Parser (Bison)         ← 🔄 In Progress
        ↓
   [3] AST + Semantic Analysis
        ↓
   [4] Computation Graph IR
        ↓
   [5] LLVM IR Generation
        ↓
   [6] Native Code (AVX2/SSE4.2 optimized)
```

## Target Hardware

- **CPU:** Intel Core i5-5200U (Broadwell)
- **SIMD:** AVX2, SSE4.2
- **Optimizations:** Cache-aware layouts, vectorized ops, multi-threaded inference

## Project Structure

```
src/
├── lexer/      # Flex tokenizer
├── parser/     # Bison grammar + AST builder
├── ast/        # AST node definitions
├── semantic/   # Type checking, validation
└── codegen/    # LLVM IR generation
examples/       # .nlang sample files
build/          # Compiled outputs
```

## Language Features

- **Networks:** Define neural network architectures
- **Modules:** Reusable sub-network blocks (e.g., InceptionBlock)
- **Layers:** Conv2D, Dense, MaxPool, AvgPool, Flatten, Concat, BatchNorm, LayerNorm
- **Dataflow:** Explicit tensor routing with `from` keyword

## Build

```bash
make          # Build compiler
make test     # Run on examples
make clean    # Clean build artifacts
```

## Example

```netlang
network MNIST {
    input(shape: [28, 28, 1])
    weights("model.bin")
    
    x = Conv2D(filters: 32, kernel: [3,3], activation: relu) from input
    x = MaxPool(pool: [2,2]) from x
    x = Flatten() from x
    x = Dense(units: 10, activation: softmax) from x
}
```

## Author

Abhijeet Deb Nath — Compiler Design Course
