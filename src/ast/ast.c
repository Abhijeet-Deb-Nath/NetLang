/*
 * NetLang AST - Implementation
 * 
 * Debug printing and memory management for AST nodes.
 * 
 * Author: Abhijeet Deb Nath
 * Course: Compiler Design
 */

#include "ast.h"

/* ========== HELPER FUNCTIONS ========== */

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static const char* node_type_name(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_NETWORK: return "NETWORK";
        case NODE_MODULE: return "MODULE";
        case NODE_INPUT: return "INPUT";
        case NODE_WEIGHTS: return "WEIGHTS";
        case NODE_STATEMENT_LIST: return "STATEMENT_LIST";
        case NODE_ASSIGNMENT: return "ASSIGNMENT";
        case NODE_RETURN: return "RETURN";
        case NODE_CONV2D: return "CONV2D";
        case NODE_DENSE: return "DENSE";
        case NODE_MAXPOOL: return "MAXPOOL";
        case NODE_AVGPOOL: return "AVGPOOL";
        case NODE_FLATTEN: return "FLATTEN";
        case NODE_ADD: return "ADD";
        case NODE_CONCAT: return "CONCAT";
        case NODE_BATCHNORM: return "BATCHNORM";
        case NODE_LAYERNORM: return "LAYERNORM";
        case NODE_MODULE_CALL: return "MODULE_CALL";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_NUMBER: return "NUMBER";
        case NODE_STRING: return "STRING";
        case NODE_ARRAY: return "ARRAY";
        case NODE_FROM_EXPR: return "FROM_EXPR";
        default: return "UNKNOWN";
    }
}

/* ========== AST PRINTING ========== */

void ast_print(ASTNode* node, int indent) {
    if (!node) {
        print_indent(indent);
        printf("(null)\n");
        return;
    }
    
    print_indent(indent);
    printf("[%s] (line %d)\n", node_type_name(node->type), node->line_number);
    
    switch (node->type) {
        case NODE_PROGRAM: {
            ASTList* defs = node->data.program.definitions;
            if (defs) {
                ASTNode* curr = defs->head;
                while (curr) {
                    ast_print(curr, indent + 1);
                    curr = curr->next;
                }
            }
            break;
        }
        
        case NODE_NETWORK: {
            print_indent(indent + 1);
            printf("name: %s\n", node->data.network.name);
            if (node->data.network.input) {
                print_indent(indent + 1);
                printf("input:\n");
                ast_print(node->data.network.input, indent + 2);
            }
            if (node->data.network.weights) {
                print_indent(indent + 1);
                printf("weights:\n");
                ast_print(node->data.network.weights, indent + 2);
            }
            if (node->data.network.statements) {
                print_indent(indent + 1);
                printf("statements:\n");
                ASTNode* curr = node->data.network.statements->head;
                while (curr) {
                    ast_print(curr, indent + 2);
                    curr = curr->next;
                }
            }
            break;
        }
        
        case NODE_MODULE: {
            print_indent(indent + 1);
            printf("name: %s\n", node->data.module.name);
            if (node->data.module.params) {
                print_indent(indent + 1);
                printf("params: ");
                Parameter* p = node->data.module.params->head;
                while (p) {
                    printf("%s", p->name);
                    if (p->next) printf(", ");
                    p = p->next;
                }
                printf("\n");
            }
            if (node->data.module.statements) {
                print_indent(indent + 1);
                printf("body:\n");
                ASTNode* curr = node->data.module.statements->head;
                while (curr) {
                    ast_print(curr, indent + 2);
                    curr = curr->next;
                }
            }
            if (node->data.module.return_stmt) {
                print_indent(indent + 1);
                printf("return:\n");
                ast_print(node->data.module.return_stmt, indent + 2);
            }
            break;
        }
        
        case NODE_INPUT: {
            print_indent(indent + 1);
            printf("shape:\n");
            ast_print(node->data.input.shape, indent + 2);
            break;
        }
        
        case NODE_WEIGHTS: {
            print_indent(indent + 1);
            printf("path: %s\n", node->data.weights.path);
            break;
        }
        
        case NODE_ASSIGNMENT: {
            print_indent(indent + 1);
            printf("target: %s\n", node->data.assignment.target);
            print_indent(indent + 1);
            printf("value:\n");
            ast_print(node->data.assignment.value, indent + 2);
            if (node->data.assignment.from_source) {
                print_indent(indent + 1);
                printf("from:\n");
                ast_print(node->data.assignment.from_source, indent + 2);
            }
            break;
        }
        
        case NODE_RETURN: {
            print_indent(indent + 1);
            printf("value:\n");
            ast_print(node->data.return_stmt.value, indent + 2);
            break;
        }
        
        case NODE_CONV2D: {
            print_indent(indent + 1);
            printf("filters: %d\n", node->data.conv2d.filters);
            print_indent(indent + 1);
            printf("kernel: [%d, %d]\n", node->data.conv2d.kernel[0], node->data.conv2d.kernel[1]);
            print_indent(indent + 1);
            printf("stride: %d\n", node->data.conv2d.stride);
            print_indent(indent + 1);
            printf("padding: %d\n", node->data.conv2d.padding);
            print_indent(indent + 1);
            printf("activation: %s\n", activation_to_string(node->data.conv2d.activation));
            break;
        }
        
        case NODE_DENSE: {
            print_indent(indent + 1);
            printf("units: %d\n", node->data.dense.units);
            print_indent(indent + 1);
            printf("activation: %s\n", activation_to_string(node->data.dense.activation));
            break;
        }
        
        case NODE_MAXPOOL:
        case NODE_AVGPOOL: {
            print_indent(indent + 1);
            printf("pool: [%d, %d]\n", node->data.pooling.pool[0], node->data.pooling.pool[1]);
            print_indent(indent + 1);
            printf("stride: %d\n", node->data.pooling.stride);
            break;
        }
        
        case NODE_ADD: {
            print_indent(indent + 1);
            printf("inputs:\n");
            if (node->data.add.inputs) {
                ASTNode* curr = node->data.add.inputs->head;
                while (curr) {
                    ast_print(curr, indent + 2);
                    curr = curr->next;
                }
            }
            break;
        }

        case NODE_CONCAT: {
            print_indent(indent + 1);
            printf("inputs:\n");
            if (node->data.concat.inputs) {
                ASTNode* curr = node->data.concat.inputs->head;
                while (curr) {
                    ast_print(curr, indent + 2);
                    curr = curr->next;
                }
            }
            break;
        }
        
        case NODE_MODULE_CALL: {
            print_indent(indent + 1);
            printf("module: %s\n", node->data.module_call.module_name);
            if (node->data.module_call.args) {
                print_indent(indent + 1);
                printf("args:\n");
                Parameter* p = node->data.module_call.args->head;
                while (p) {
                    print_indent(indent + 2);
                    printf("%s:\n", p->name);
                    ast_print(p->value, indent + 3);
                    p = p->next;
                }
            }
            break;
        }
        
        case NODE_IDENTIFIER: {
            print_indent(indent + 1);
            printf("name: %s\n", node->data.identifier.name);
            break;
        }
        
        case NODE_NUMBER: {
            print_indent(indent + 1);
            if (node->data.number.is_float) {
                printf("value: %f (float)\n", node->data.number.fval);
            } else {
                printf("value: %d (int)\n", node->data.number.ival);
            }
            break;
        }
        
        case NODE_STRING: {
            print_indent(indent + 1);
            printf("value: \"%s\"\n", node->data.string.value);
            break;
        }
        
        case NODE_ARRAY: {
            print_indent(indent + 1);
            printf("elements:\n");
            if (node->data.array.elements) {
                ASTNode* curr = node->data.array.elements->head;
                while (curr) {
                    ast_print(curr, indent + 2);
                    curr = curr->next;
                }
            }
            break;
        }
        
        case NODE_FLATTEN:
        case NODE_BATCHNORM:
        case NODE_LAYERNORM:
            /* No additional data */
            break;
            
        default:
            print_indent(indent + 1);
            printf("(unhandled node type)\n");
            break;
    }
}

/* ========== MEMORY CLEANUP ========== */

static void free_parameter_list(ParameterList* list) {
    if (!list) return;
    Parameter* curr = list->head;
    while (curr) {
        Parameter* next = curr->next;
        free(curr->name);
        ast_free(curr->value);
        free(curr);
        curr = next;
    }
    free(list);
}

static void free_ast_list(ASTList* list) {
    if (!list) return;
    ASTNode* curr = list->head;
    while (curr) {
        ASTNode* next = curr->next;
        ast_free(curr);
        curr = next;
    }
    free(list);
}

void ast_free(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            free_ast_list(node->data.program.definitions);
            break;
            
        case NODE_NETWORK:
            free(node->data.network.name);
            ast_free(node->data.network.input);
            ast_free(node->data.network.weights);
            free_ast_list(node->data.network.statements);
            break;
            
        case NODE_MODULE:
            free(node->data.module.name);
            free_parameter_list(node->data.module.params);
            free_ast_list(node->data.module.statements);
            ast_free(node->data.module.return_stmt);
            break;
            
        case NODE_INPUT:
            ast_free(node->data.input.shape);
            break;
            
        case NODE_WEIGHTS:
            free(node->data.weights.path);
            break;
            
        case NODE_ASSIGNMENT:
            free(node->data.assignment.target);
            ast_free(node->data.assignment.value);
            ast_free(node->data.assignment.from_source);
            break;
            
        case NODE_RETURN:
            ast_free(node->data.return_stmt.value);
            break;
            
        case NODE_ADD:
            free_ast_list(node->data.add.inputs);
            break;

        case NODE_CONCAT:
            free_ast_list(node->data.concat.inputs);
            break;
            
        case NODE_MODULE_CALL:
            free(node->data.module_call.module_name);
            free_parameter_list(node->data.module_call.args);
            break;
            
        case NODE_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case NODE_STRING:
            free(node->data.string.value);
            break;
            
        case NODE_ARRAY:
            free_ast_list(node->data.array.elements);
            break;
            
        case NODE_FROM_EXPR:
            ast_free(node->data.from_expr.layer);
            ast_free(node->data.from_expr.source);
            break;
            
        default:
            /* No dynamic memory */
            break;
    }
    
    free(node);
}
