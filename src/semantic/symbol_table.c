#include "symbol_table.h"
#include "type_checker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration */
static const char* symbol_type_name(SymbolType type);

Scope* scope_create(const char* name, Scope* parent) {
    Scope* scope = (Scope*)calloc(1, sizeof(Scope));
    scope->name = strdup(name);
    scope->parent = parent;
    return scope;
}

Symbol* scope_lookup(Scope* scope, const char* name, int recursive) {
    // Search current scope
    Symbol* sym = scope->symbols;
    while (sym) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
        sym = sym->next;
    }
    
    // Recursively search parent scopes
    if (recursive && scope->parent) {
        return scope_lookup(scope->parent, name, 1);
    }
    
    return NULL;
}

Symbol* scope_insert(Scope* scope, const char* name, SymbolType type, 
                     ASTNode* node, int line) {
    Symbol* sym = (Symbol*)calloc(1, sizeof(Symbol));
    sym->name = strdup(name);
    sym->sym_type = type;
    sym->ast_node = node;
    sym->line = line;
    
    // Add to front of list
    sym->next = scope->symbols;
    scope->symbols = sym;
    
    return sym;
}

void scope_destroy(Scope* scope) {
    if (!scope) return;
    
    // Free all symbols
    Symbol* sym = scope->symbols;
    while (sym) {
        Symbol* next = sym->next;
        free(sym->name);
        tensor_type_free(sym->tensor_type);
        free(sym);
        sym = next;
    }
    
    // Free child scopes
    Scope* child = scope->children;
    while (child) {
        Scope* next = child->next;
        scope_destroy(child);
        child = next;
    }
    
    // Free scope itself
    free(scope->name);
    free(scope);
}

void scope_print(Scope* scope, int indent) {
    if (!scope) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    printf("Scope: %s\n", scope->name);
    
    // Print symbols
    Symbol* sym = scope->symbols;
    while (sym) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("- %s (%s)", sym->name, symbol_type_name(sym->sym_type));
        
        if (sym->tensor_type) {
            printf(" [");
            for (int i = 0; i < sym->tensor_type->rank; i++) {
                printf("%d", sym->tensor_type->dims[i]);
                if (i < sym->tensor_type->rank - 1) printf(", ");
            }
            printf("]");
        }
        printf("\n");
        
        sym = sym->next;
    }
    
    // Print child scopes
    Scope* child = scope->children;
    while (child) {
        scope_print(child, indent + 1);
        child = child->next;
    }
}

static const char* symbol_type_name(SymbolType type) {
    switch (type) {
        case SYM_VARIABLE: return "VAR";
        case SYM_INPUT: return "INPUT";
        case SYM_WEIGHTS: return "WEIGHTS";
        case SYM_LAYER: return "LAYER";
        case SYM_MODULE: return "MODULE";
        default: return "UNKNOWN";
    }
}