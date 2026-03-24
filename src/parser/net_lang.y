%{
/*
 * NetLang Parser - Phase 2 & 3
 * Builds Abstract Syntax Tree from token stream
 * Performs semantic analysis and type checking
 * 
 * Grammar for NetLang DSL:
 * - Networks: network Name { ... }
 * - Modules: module Name(params) { ... return expr }
 * - Layers: Conv2D, Dense, MaxPool, etc.
 * - Dataflow: Layer(...) from source
 * 
 * Author: Abhijeet Deb Nath
 * Course: Compiler Design
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"
#include "../semantic/semantic.h"

/* Flex/Bison integration */
extern int yylex(void);
extern int yylineno;
extern char* yytext;
extern FILE* yyin;

void yyerror(const char* s);

static int require_int_literal(ASTNode* node,
                               const char* what,
                               int line,
                               int allow_zero,
                               int allow_negative,
                               int* out_value) {
    if (!node || node->type != NODE_NUMBER || node->data.number.is_float) {
        fprintf(stderr, "Parse error at line %d: %s must be an integer literal\n",
                line, what);
        return 0;
    }

    int value = node->data.number.ival;
    if ((!allow_negative && value < 0) ||
        (!allow_zero && value == 0)) {
        fprintf(stderr, "Parse error at line %d: %s must be %s\n",
                line,
                what,
                allow_negative ? "non-zero" : (allow_zero ? "non-negative" : "positive"));
        return 0;
    }

    *out_value = value;
    return 1;
}

static int require_int_pair(ASTNode* node,
                            const char* what,
                            int line,
                            int allow_zero,
                            int out_values[2]) {
    if (!node || node->type != NODE_ARRAY || !node->data.array.elements) {
        fprintf(stderr, "Parse error at line %d: %s must be a two-element integer array\n",
                line, what);
        return 0;
    }

    if (node->data.array.elements->count != 2) {
        fprintf(stderr, "Parse error at line %d: %s must contain exactly two integers\n",
                line, what);
        return 0;
    }

    ASTNode* elem = node->data.array.elements->head;
    for (int i = 0; i < 2; i++) {
        if (!require_int_literal(elem, what, line, allow_zero, 0, &out_values[i])) {
            return 0;
        }
        elem = elem->next;
    }

    return 1;
}

/* Global AST root - populated after successful parse */
ASTNode* ast_root = NULL;

%}

/* Modern syntax for verbose errors */
%define parse.error verbose

/* Track source locations */
%locations

/* ========== SEMANTIC VALUE TYPES ========== */
%union {
    int ival;
    float fval;
    char* sval;
    ASTNode* node;
    ASTList* list;
    ParameterList* params;
    ActivationType activation;
    NetworkBody* netbody;
}

/* ========== TOKEN DECLARATIONS ========== */

/* Keywords */
%token NETWORK MODULE RETURN FROM INPUT SHAPE WEIGHTS

/* Layer types */
%token CONV2D DENSE MAXPOOL AVGPOOL FLATTEN ADD CONCAT BATCHNORM LAYERNORM

/* Parameter keywords */
%token FILTERS KERNEL ACTIVATION STRIDE PADDING POOL UNITS

/* Activation functions */
%token RELU SIGMOID TANH SOFTMAX LINEAR

/* Literals */
%token <ival> NUMBER
%token <fval> FLOAT_NUM
%token <sval> IDENTIFIER STRING_LIT

/* Punctuation */
%token LBRACE RBRACE LBRACKET RBRACKET LPAREN RPAREN
%token COLON COMMA ASSIGN

/* ========== NON-TERMINAL TYPES ========== */
%type <node> program network_def module_def
%type <node> input_decl weights_decl optional_weights_decl
%type <node> statement assignment return_stmt
%type <node> layer_expr conv2d_layer dense_layer pool_layer
%type <node> flatten_layer add_layer concat_layer norm_layer module_call
%type <node> expr optional_from_clause identifier_expr
%type <node> number array
%type <list> module_list statement_list statement_list_nonempty array_elements add_args concat_args
%type <params> module_params param_list layer_params layer_param_list
%type <activation> activation_value
%type <netbody> network_body

/* ========== PRECEDENCE ========== */
%left FROM

%%

/* ========== GRAMMAR RULES ========== */

/* ONE network per file, with optional helper modules */
program:
    network_def {
        ast_root = ast_program(@1.first_line);
        ASTList* defs = ast_list_new();
        ast_list_append(defs, $1);
        ast_root->data.program.definitions = defs;
        $$ = ast_root;
    }
    | module_list network_def {
        ast_root = ast_program(@1.first_line);
        ast_list_append($1, $2);  /* Add network to end of module list */
        ast_root->data.program.definitions = $1;
        $$ = ast_root;
    }
    ;

/* Zero or more modules (must come before network) */
module_list:
    module_def {
        $$ = ast_list_new();
        ast_list_append($$, $1);
    }
    | module_list module_def {
        ast_list_append($1, $2);
        $$ = $1;
    }
    ;

/* ========== NETWORK DEFINITION ========== */

network_def:
    NETWORK IDENTIFIER LBRACE network_body RBRACE {
        $$ = ast_node_new(NODE_NETWORK, @1.first_line);
        $$->data.network.name = $2;  /* Transfer ownership, don't free */
        $$->data.network.input = $4->input;
        $$->data.network.weights = $4->weights;
        $$->data.network.statements = $4->statements;
        free($4);  /* Free the temporary NetworkBody struct only */
    }
    ;

network_body:
    input_decl optional_weights_decl statement_list_nonempty {
        $$ = (NetworkBody*)ast_alloc(sizeof(NetworkBody));
        $$->input = $1;
        $$->weights = $2;
        $$->statements = $3;
    }
    ;

/* ========== MODULE DEFINITION ========== */

module_def:
    MODULE IDENTIFIER LPAREN module_params RPAREN LBRACE statement_list return_stmt RBRACE {
        $$ = ast_module($2, $4, @1.first_line);
        $$->data.module.statements = $7;
        $$->data.module.return_stmt = $8;
        free($2);
    }
    ;

module_params:
    /* empty */ {
        $$ = param_list_new();
    }
    | param_list {
        $$ = $1;
    }
    ;

param_list:
    IDENTIFIER {
        $$ = param_list_new();
        param_list_add($$, $1, NULL);
        free($1);
    }
    | param_list COMMA IDENTIFIER {
        param_list_add($1, $3, NULL);
        $$ = $1;
        free($3);
    }
    ;

/* ========== INPUT & WEIGHTS ========== */

input_decl:
    INPUT LPAREN SHAPE COLON array RPAREN {
        $$ = ast_input($5, @1.first_line);
    }
    ;

weights_decl:
    WEIGHTS LPAREN STRING_LIT RPAREN {
        $$ = ast_weights($3, @1.first_line);
        free($3);
    }
    ;

optional_weights_decl:
    /* empty */ {
        $$ = NULL;
    }
    | weights_decl {
        $$ = $1;
    }
    ;

/* ========== STATEMENTS ========== */

statement_list:
    /* empty */ {
        $$ = ast_list_new();
    }
    | statement_list statement {
        ast_list_append($1, $2);
        $$ = $1;
    }
    ;

statement_list_nonempty:
    statement {
        $$ = ast_list_new();
        ast_list_append($$, $1);
    }
    | statement_list_nonempty statement {
        ast_list_append($1, $2);
        $$ = $1;
    }
    ;

statement:
    assignment { $$ = $1; }
    ;

assignment:
    IDENTIFIER ASSIGN layer_expr optional_from_clause {
        $$ = ast_assignment($1, $3, $4, @1.first_line);
        free($1);
    }
    ;

optional_from_clause:
    /* empty */ {
        $$ = NULL;
    }
    |
    FROM expr {
        $$ = $2;
    }
    ;

return_stmt:
    RETURN expr {
        ASTNode* node = ast_node_new(NODE_RETURN, @1.first_line);
        node->data.return_stmt.value = $2;
        $$ = node;
    }
    ;

/* ========== LAYER EXPRESSIONS ========== */

layer_expr:
    conv2d_layer { $$ = $1; }
    | dense_layer { $$ = $1; }
    | pool_layer { $$ = $1; }
    | flatten_layer { $$ = $1; }
    | add_layer { $$ = $1; }
    | concat_layer { $$ = $1; }
    | norm_layer { $$ = $1; }
    | module_call { $$ = $1; }
    ;

/* ========== CONV2D LAYER ========== */

conv2d_layer:
    CONV2D LPAREN 
        FILTERS COLON expr COMMA 
        KERNEL COLON array COMMA 
        STRIDE COLON expr COMMA 
        PADDING COLON expr COMMA 
        ACTIVATION COLON activation_value 
    RPAREN {
        ASTNode* node = ast_node_new(NODE_CONV2D, @1.first_line);

        if (!require_int_literal($5, "filters", @5.first_line, 0, 0, &node->data.conv2d.filters) ||
            !require_int_pair($9, "kernel", @9.first_line, 0, node->data.conv2d.kernel) ||
            !require_int_literal($13, "stride", @13.first_line, 0, 0, &node->data.conv2d.stride) ||
            !require_int_literal($17, "padding", @17.first_line, 1, 0, &node->data.conv2d.padding)) {
            YYERROR;
        }

        node->data.conv2d.activation = $21;

        $$ = node;
    }
    ;

/* ========== DENSE LAYER ========== */

dense_layer:
    DENSE LPAREN 
        UNITS COLON expr COMMA 
        ACTIVATION COLON activation_value 
    RPAREN {
        ASTNode* node = ast_node_new(NODE_DENSE, @1.first_line);

        if (!require_int_literal($5, "units", @5.first_line, 0, 0, &node->data.dense.units)) {
            YYERROR;
        }

        node->data.dense.activation = $9;

        $$ = node;
    }
    ;

/* ========== POOLING LAYERS ========== */

pool_layer:
    MAXPOOL LPAREN 
        POOL COLON array COMMA 
        STRIDE COLON expr COMMA 
        PADDING COLON expr 
    RPAREN {
        ASTNode* node = ast_node_new(NODE_MAXPOOL, @1.first_line);

        if (!require_int_pair($5, "pool", @5.first_line, 0, node->data.pooling.pool) ||
            !require_int_literal($9, "stride", @9.first_line, 1, 0, &node->data.pooling.stride) ||
            !require_int_literal($13, "padding", @13.first_line, 1, 0, &node->data.pooling.padding)) {
            YYERROR;
        }

        $$ = node;
    }
    | AVGPOOL LPAREN 
        POOL COLON array COMMA 
        STRIDE COLON expr COMMA 
        PADDING COLON expr 
    RPAREN {
        ASTNode* node = ast_node_new(NODE_AVGPOOL, @1.first_line);

        if (!require_int_pair($5, "pool", @5.first_line, 0, node->data.pooling.pool) ||
            !require_int_literal($9, "stride", @9.first_line, 1, 0, &node->data.pooling.stride) ||
            !require_int_literal($13, "padding", @13.first_line, 1, 0, &node->data.pooling.padding)) {
            YYERROR;
        }

        $$ = node;
    }
    ;

/* ========== OTHER LAYERS ========== */

flatten_layer:
    FLATTEN LPAREN RPAREN {
        $$ = ast_node_new(NODE_FLATTEN, @1.first_line);
    }
    ;

add_layer:
    ADD LPAREN add_args RPAREN {
        ASTNode* node = ast_node_new(NODE_ADD, @1.first_line);
        node->data.add.inputs = $3;
        $$ = node;
    }
    ;

add_args:
    expr {
        $$ = ast_list_new();
        ast_list_append($$, $1);
    }
    | add_args COMMA expr {
        ast_list_append($1, $3);
        $$ = $1;
    }
    ;

concat_layer:
    CONCAT LPAREN concat_args RPAREN {
        ASTNode* node = ast_node_new(NODE_CONCAT, @1.first_line);
        node->data.concat.inputs = $3;
        $$ = node;
    }
    ;

concat_args:
    expr {
        $$ = ast_list_new();
        ast_list_append($$, $1);
    }
    | concat_args COMMA expr {
        ast_list_append($1, $3);
        $$ = $1;
    }
    ;

norm_layer:
    BATCHNORM LPAREN RPAREN {
        $$ = ast_node_new(NODE_BATCHNORM, @1.first_line);
    }
    | LAYERNORM LPAREN RPAREN {
        $$ = ast_node_new(NODE_LAYERNORM, @1.first_line);
    }
    ;

/* ========== MODULE CALL ========== */

module_call:
    IDENTIFIER LPAREN layer_params RPAREN {
        ASTNode* node = ast_node_new(NODE_MODULE_CALL, @1.first_line);
        node->data.module_call.module_name = strdup($1);
        node->data.module_call.args = $3;
        free($1);
        $$ = node;
    }
    ;

/* ========== LAYER PARAMETERS ========== */

layer_params:
    /* empty */ {
        $$ = param_list_new();
    }
    | layer_param_list {
        $$ = $1;
    }
    ;

layer_param_list:
    IDENTIFIER COLON expr {
        $$ = param_list_new();
        param_list_add($$, $1, $3);
        free($1);
    }
    /* Parameter keywords */
    | FILTERS COLON expr {
        $$ = param_list_new();
        param_list_add($$, "filters", $3);
    }
    | KERNEL COLON expr {
        $$ = param_list_new();
        param_list_add($$, "kernel", $3);
    }
    | ACTIVATION COLON expr {
        $$ = param_list_new();
        param_list_add($$, "activation", $3);
    }
    | STRIDE COLON expr {
        $$ = param_list_new();
        param_list_add($$, "stride", $3);
    }
    | PADDING COLON expr {
        $$ = param_list_new();
        param_list_add($$, "padding", $3);
    }
    | POOL COLON expr {
        $$ = param_list_new();
        param_list_add($$, "pool", $3);
    }
    | UNITS COLON expr {
        $$ = param_list_new();
        param_list_add($$, "units", $3);
    }
    | layer_param_list COMMA IDENTIFIER COLON expr {
        param_list_add($1, $3, $5);
        free($3);
        $$ = $1;
    }
    | layer_param_list COMMA FILTERS COLON expr {
        param_list_add($1, "filters", $5);
        $$ = $1;
    }
    | layer_param_list COMMA KERNEL COLON expr {
        param_list_add($1, "kernel", $5);
        $$ = $1;
    }
    | layer_param_list COMMA ACTIVATION COLON expr {
        param_list_add($1, "activation", $5);
        $$ = $1;
    }
    | layer_param_list COMMA STRIDE COLON expr {
        param_list_add($1, "stride", $5);
        $$ = $1;
    }
    | layer_param_list COMMA PADDING COLON expr {
        param_list_add($1, "padding", $5);
        $$ = $1;
    }
    | layer_param_list COMMA POOL COLON expr {
        param_list_add($1, "pool", $5);
        $$ = $1;
    }
    | layer_param_list COMMA UNITS COLON expr {
        param_list_add($1, "units", $5);
        $$ = $1;
    }
    ;

/* ========== EXPRESSIONS ========== */

expr:
    number { $$ = $1; }
    | array { $$ = $1; }
    | identifier_expr { $$ = $1; }
    | activation_value {
        /* Convert activation token to identifier node */
        $$ = ast_identifier(activation_to_string($1), @1.first_line);
    }
    ;

identifier_expr:
    IDENTIFIER {
        $$ = ast_identifier($1, @1.first_line);
        free($1);
    }
    | INPUT {
        $$ = ast_identifier("input", @1.first_line);
    }
    ;

number:
    NUMBER {
        $$ = ast_number_int($1, @1.first_line);
    }
    | FLOAT_NUM {
        $$ = ast_number_float($1, @1.first_line);
    }
    ;

array:
    LBRACKET array_elements RBRACKET {
        ASTNode* node = ast_array(@1.first_line);
        node->data.array.elements = $2;
        $$ = node;
    }
    ;

array_elements:
    number {
        $$ = ast_list_new();
        ast_list_append($$, $1);
    }
    | array_elements COMMA number {
        ast_list_append($1, $3);
        $$ = $1;
    }
    ;

activation_value:
    RELU { $$ = ACT_RELU; }
    | SIGMOID { $$ = ACT_SIGMOID; }
    | TANH { $$ = ACT_TANH; }
    | SOFTMAX { $$ = ACT_SOFTMAX; }
    | LINEAR { $$ = ACT_LINEAR; }
    ;

%%

/* ========== ERROR HANDLING ========== */

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
    fprintf(stderr, "Near token: '%s'\n", yytext);
}

/* ========== MAIN FUNCTION ========== */

#ifndef NETLANG_NO_PARSER_MAIN
/* Test main - only compiled when not using external main.c */

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.nlang>\n", argv[0]);
        return 1;
    }
    
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }
    
    printf("NetLang Parser v0.1\n");
    printf("Parsing: %s\n", argv[1]);
    printf("========================================\n\n");
    
    int result = yyparse();
    
    fclose(yyin);
    
    if (result == 0 && ast_root) {
        printf("\n✓ Parse successful!\n\n");
        printf("========== AST DUMP ==========\n\n");
        ast_print(ast_root, 0);
        
        printf("\n========================================\n");
        printf("========== SEMANTIC ANALYSIS ==========\n");
        printf("========================================\n\n");
        
        SemanticResult sem_result = analyze_program(ast_root);
        
        if (sem_result.is_valid) {
            printf("\n✓ Semantic analysis passed!\n");
            printf("  Warnings: %d\n", sem_result.warning_count);
            
            printf("\n========== SYMBOL TABLE ==========\n\n");
            scope_print(sem_result.global_scope, 0);
        } else {
            fprintf(stderr, "\n✗ Semantic analysis failed!\n");
            fprintf(stderr, "  Errors: %d\n", sem_result.error_count);
            fprintf(stderr, "  Warnings: %d\n", sem_result.warning_count);
            
            /* Cleanup */
            scope_destroy(sem_result.global_scope);
            ast_free(ast_root);
            return 1;
        }
        
        /* Cleanup */
        scope_destroy(sem_result.global_scope);
        ast_free(ast_root);
    } else {
        fprintf(stderr, "\n✗ Parse failed with errors.\n");
        return 1;
    }
    
    return 0;
}

#endif /* NETLANG_NO_PARSER_MAIN */
