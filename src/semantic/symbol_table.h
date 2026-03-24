#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../ast/ast.h"

typedef enum {
    SYM_VARIABLE,   // x, y, b1, etc.
    SYM_INPUT,      // input tensor
    SYM_WEIGHTS,    // weights file
    SYM_LAYER,      // Layer definition
    SYM_MODULE      // Module definition
} SymbolType;

typedef struct {
    int dims[4];           // [height, width, channels] or [features]
    int rank;              // 1=vector, 3=tensor3d (CNN layers)
    const char* dtype;     // "float32", "int32", etc.
} TensorType;

typedef struct Symbol {
    char* name;
    SymbolType sym_type;
    TensorType* tensor_type;  // NULL if not a tensor
    ASTNode* ast_node;        // Reference to AST node
    int line;                 // Source line for error messages
    struct Symbol* next;
} Symbol;

typedef struct Scope {
    char* name;             // "global", "network:MNIST", "module:Inception"
    Symbol* symbols;        // Linked list of symbols
    struct Scope* parent;   // Parent scope
    struct Scope* children; // Child scopes
    struct Scope* next;     // Sibling scopes
} Scope;

// API
Scope* scope_create(const char* name, Scope* parent);
void scope_destroy(Scope* scope);
Symbol* scope_lookup(Scope* scope, const char* name, int recursive);
Symbol* scope_insert(Scope* scope, const char* name, SymbolType type, 
                     ASTNode* node, int line);
Symbol* scope_bind_variable(Scope* scope, const char* name, ASTNode* node, int line);
void scope_print(Scope* scope, int indent);

#endif
