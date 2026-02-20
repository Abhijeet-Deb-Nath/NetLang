/*
 * NetLang Compiler - Main Driver
 * 
 * Compiles .nlang network definitions to high-performance C code.
 * 
 * Usage: netlang input.nlang [-o output.c]
 * 
 * Compilation stages:
 * 1. Lexical analysis (tokenization)
 * 2. Syntax analysis (parsing to AST)
 * 3. Semantic analysis (type checking, shape inference)
 * 4. Code generation (optimized C output)
 * 
 * Author: Abhijeet Deb Nath
 * Date: February 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast/ast.h"
#include "semantic/semantic.h"
#include "codegen/codegen.h"

/* External declarations from lexer/parser */
extern FILE* yyin;
extern int yyparse(void);
extern ASTNode* ast_root;      /* Set by parser */

/* Print usage information */
void print_usage(const char* program_name) {
    fprintf(stderr, "NetLang Compiler v1.0\n");
    fprintf(stderr, "High-Performance CNN Inference Code Generator\n\n");
    fprintf(stderr, "Usage: %s <input.nlang> [-o output.c]\n\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o <file>    Output file (default: stdout)\n");
    fprintf(stderr, "  -h, --help   Show this help message\n\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s examples/mnist.nlang -o generated_mnist.c\n", program_name);
    fprintf(stderr, "  gcc -O3 -march=haswell -mavx2 -mfma generated_mnist.c \\\n");
    fprintf(stderr, "      src/codegen/runtime.c src/codegen/kernels.c -o mnist\n");
}

/* Main compiler driver */
int main(int argc, char** argv) {
    const char* input_file = NULL;
    const char* output_file = NULL;
    FILE* output = stdout;
    
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: -o requires output filename\n");
                return 1;
            }
            output_file = argv[++i];
        } else if (input_file == NULL) {
            input_file = argv[i];
        } else {
            fprintf(stderr, "ERROR: Unexpected argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (input_file == NULL) {
        fprintf(stderr, "ERROR: No input file specified\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    /* ========== STAGE 1: LEXICAL + SYNTAX ANALYSIS ========== */
    fprintf(stderr, "[1/4] Parsing %s...\n", input_file);
    
    yyin = fopen(input_file, "r");
    if (!yyin) {
        fprintf(stderr, "ERROR: Cannot open input file: %s\n", input_file);
        return 1;
    }
    
    /* Run parser (also runs lexer internally) */
    ast_root = NULL;
    int parse_result = yyparse();
    fclose(yyin);
    
    if (parse_result != 0 || ast_root == NULL) {
        fprintf(stderr, "ERROR: Parsing failed\n");
        return 1;
    }
    
    fprintf(stderr, "      ✓ Parsing successful\n");
    
    /* ========== STAGE 2: SEMANTIC ANALYSIS ========== */
    fprintf(stderr, "[2/4] Running semantic analysis...\n");
    
    SemanticResult sem_result = analyze_program(ast_root);
    
    if (!sem_result.is_valid || sem_result.error_count > 0) {
        fprintf(stderr, "ERROR: Semantic analysis failed with %d error(s), %d warning(s)\n",
                sem_result.error_count, sem_result.warning_count);
        return 1;
    }
    
    fprintf(stderr, "      ✓ Semantic analysis passed\n");
    if (sem_result.warning_count > 0) {
        fprintf(stderr, "      ⚠ %d warning(s)\n", sem_result.warning_count);
    }
    
    /* ========== STAGE 3: CODE GENERATION ========== */
    fprintf(stderr, "[3/4] Generating optimized C code...\n");
    
    /* Open output file if specified */
    if (output_file != NULL) {
        output = fopen(output_file, "w");
        if (!output) {
            fprintf(stderr, "ERROR: Cannot open output file: %s\n", output_file);
            return 1;
        }
        fprintf(stderr, "      Output: %s\n", output_file);
    } else {
        fprintf(stderr, "      Output: stdout\n");
    }
    
    /* Find network node (assume first definition is a network) */
    ASTNode* network = NULL;
    if (ast_root->type == NODE_PROGRAM) {
        ASTList* defs = ast_root->data.program.definitions;
        if (defs && defs->head) {
            network = defs->head;
            if (network->type != NODE_NETWORK) {
                fprintf(stderr, "ERROR: First definition must be a network\n");
                return 1;
            }
        }
    }
    
    if (!network) {
        fprintf(stderr, "ERROR: No network found in input file\n");
        return 1;
    }
    
    /* Generate code */
    generate_network_code(network, sem_result.global_scope, output);
    
    if (output_file != NULL) {
        fclose(output);
    }
    
    fprintf(stderr, "      ✓ Code generation complete\n");
    
    /* ========== STAGE 4: COMPILATION INSTRUCTIONS ========== */
    fprintf(stderr, "[4/4] Next steps:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  Compile the generated code:\n");
    if (output_file != NULL) {
        fprintf(stderr, "    gcc -O3 -march=haswell -mavx2 -mfma \\\n");
        fprintf(stderr, "        %s \\\n", output_file);
        fprintf(stderr, "        src/codegen/runtime.c \\\n");
        fprintf(stderr, "        src/codegen/kernels.c \\\n");
        fprintf(stderr, "        -o network\n\n");
        fprintf(stderr, "  Run the network:\n");
        fprintf(stderr, "    ./network\n\n");
    } else {
        fprintf(stderr, "    Redirect output to file first:\n");
        fprintf(stderr, "      %s %s -o output.c\n", argv[0], input_file);
    }
    
    fprintf(stderr, "✓ Compilation successful!\n");
    
    return 0;
}
