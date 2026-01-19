# NetLang Lexer

A lexical analyzer for NetLang, a domain-specific language (DSL) for defining neural network architectures.

## Overview

This project implements a Flex-based lexer that tokenizes NetLang source files into categorized token streams. NetLang allows users to define CNN architectures with a clean, declarative syntax.

## Features

- **Keywords**: `network`, `input`, `shape`, `weights`, `layers`
- **Layer types**: `Conv2D`, `Dense`, `MaxPool`, `AvgPool`, `Flatten`
- **Value types**: Numbers, identifiers, strings (generic, activation functions, file paths)
- **Error handling**: Line tracking and error reporting

## Usage

1. **Compile the lexer:**
   ```bash
   flex net_lang.l
   gcc lex.yy.c -o lexer
   ```

2. **Tokenize a NetLang file:**
   ```bash
   ./lexer < demo.nlang
   ```

## Example NetLang Code

```
network MNIST_CNN {
    input: { shape: [28, 28, 1] }
    weights: "models/mnist_cnn.bin"
    
    layers: [
        Conv2D(filters: 32, kernel: [3, 3], activation: "relu")
        MaxPool(pool: 2)
        Flatten()
        Dense(units: 10, activation: "softmax")
    ]
}
```

## Files

- `net_lang.l` - Flex lexer specification
- `demo.nlang` - Example NetLang programs
- `lex.yy.c` - Generated C lexer code
- `demo.nlang_tokens.txt` - Sample tokenized output

## Author

Compiler Design Course Project
