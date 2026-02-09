#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../ast/ast.h"
#include "symbol_table.h"
#include "type_checker.h"

typedef struct SemanticResult {
    int error_count;
    int warning_count;
    Scope* global_scope;
    int is_valid;
} SemanticResult;

// Main entry point
SemanticResult analyze_program(ASTNode* program);

// Individual analyzers
void analyze_network(TypeChecker* tc, ASTNode* network);
void analyze_module(TypeChecker* tc, ASTNode* module);
void analyze_statement(TypeChecker* tc, ASTNode* stmt);
void analyze_expression(TypeChecker* tc, ASTNode* expr);

// Error reporting
void semantic_error(TypeChecker* tc, int line, const char* fmt, ...);
void semantic_warning(TypeChecker* tc, int line, const char* fmt, ...);

#endif