#include "semantic.h"
#include <stdarg.h>
#include <stdio.h>

/* ========== ERROR REPORTING ========== */

void semantic_error(TypeChecker* tc, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    fprintf(tc->error_stream, "Error (line %d): ", line);
    vfprintf(tc->error_stream, fmt, args);
    fprintf(tc->error_stream, "\n");
    
    va_end(args);
    tc->error_count++;
}

void semantic_warning(TypeChecker* tc, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    fprintf(tc->error_stream, "Warning (line %d): ", line);
    vfprintf(tc->error_stream, fmt, args);
    fprintf(tc->error_stream, "\n");
    
    va_end(args);
    tc->warning_count++;
}

/* ========== MAIN ANALYSIS ========== */

SemanticResult analyze_program(ASTNode* program) {
    SemanticResult result = {0};
    
    if (!program || program->type != NODE_PROGRAM) {
        fprintf(stderr, "FATAL: Invalid AST root\n");
        result.is_valid = 0;
        return result;
    }
    
    // Create type checker with global scope
    TypeChecker tc = {
        .current_scope = scope_create("global", NULL),
        .error_count = 0,
        .warning_count = 0,
        .error_stream = stderr
    };
    
    // Analyze each definition (network or module)
    ASTList* defs = program->data.program.definitions;
    ASTNode* def = defs->head;
    
    while (def) {
        switch (def->type) {
            case NODE_NETWORK:
                analyze_network(&tc, def);
                break;
            case NODE_MODULE:
                analyze_module(&tc, def);
                break;
            default:
                semantic_error(&tc, def->line_number,
                             "Unexpected definition type");
        }
        def = def->next;
    }
    
    result.error_count = tc.error_count;
    result.warning_count = tc.warning_count;
    result.global_scope = tc.current_scope;
    result.is_valid = (tc.error_count == 0);
    
    return result;
}

void analyze_network(TypeChecker* tc, ASTNode* network) {
    char scope_name[256];
    snprintf(scope_name, sizeof(scope_name), "network:%s", 
             network->data.network.name);
    
    // Add network to global scope
    scope_insert(tc->current_scope, network->data.network.name,
                SYM_LAYER, network, network->line_number);
    
    // Create new scope for this network
    Scope* network_scope = scope_create(scope_name, tc->current_scope);
    
    // Link child scope to parent
    if (!tc->current_scope->children) {
        tc->current_scope->children = network_scope;
    } else {
        Scope* sibling = tc->current_scope->children;
        while (sibling->next) {
            sibling = sibling->next;
        }
        sibling->next = network_scope;
    }
    
    tc->current_scope = network_scope;
    
    // Add input symbol
    if (network->data.network.input) {
        ASTNode* input_node = network->data.network.input;
        Symbol* input_sym = scope_insert(tc->current_scope, "input", 
                                        SYM_INPUT, input_node,
                                        input_node->line_number);
        
        // Infer input shape from array node
        // User specifies [H, W, C] - 3D tensor for single-image inference
        ASTNode* shape_array = input_node->data.input.shape;
        if (shape_array && shape_array->type == NODE_ARRAY) {
            ASTList* elements = shape_array->data.array.elements;
            if (elements->count == 3) {
                // Extract dimensions from array [H, W, C]
                int dims[3] = {0, 0, 0};
                int idx = 0;
                
                ASTNode* elem = elements->head;
                while (elem && idx < 3) {
                    if (elem->type == NODE_NUMBER && !elem->data.number.is_float) {
                        dims[idx++] = elem->data.number.ival;
                    }
                    elem = elem->next;
                }
                
                // Create 3D tensor type [H, W, C]
                TensorType* input_type = tensor_type_create(3, dims);
                input_sym->tensor_type = input_type;
            } else {
                semantic_error(tc, input_node->line_number,
                             "Input shape must have 3 dimensions [H, W, C]");
            }
        }
    }
    
    // Process statements
    ASTList* stmts = network->data.network.statements;
    ASTNode* stmt = stmts->head;
    while (stmt) {
        analyze_statement(tc, stmt);
        stmt = stmt->next;
    }
    
    // Return to parent scope
    tc->current_scope = network_scope->parent;
}

void analyze_module(TypeChecker* tc, ASTNode* module) {
    char scope_name[256];
    snprintf(scope_name, sizeof(scope_name), "module:%s", 
             module->data.module.name);
    
    // Add module to global scope for lookup
    scope_insert(tc->current_scope, 
                module->data.module.name,
                SYM_MODULE, 
                module,
                module->line_number);
    
    // Create scope for module body
    Scope* module_scope = scope_create(scope_name, tc->current_scope);
    
    // Link child scope to parent
    if (!tc->current_scope->children) {
        tc->current_scope->children = module_scope;
    } else {
        Scope* sibling = tc->current_scope->children;
        while (sibling->next) {
            sibling = sibling->next;
        }
        sibling->next = module_scope;
    }
    
    tc->current_scope = module_scope;
    
    // Add parameters to module scope
    if (module->data.module.params) {
        Parameter* param = module->data.module.params->head;
        while (param) {
            scope_insert(tc->current_scope, param->name, 
                        SYM_VARIABLE, NULL, module->line_number);
            param = param->next;
        }
    }
    
    // Analyze module body
    if (module->data.module.statements) {
        ASTNode* stmt = module->data.module.statements->head;
        while (stmt) {
            analyze_statement(tc, stmt);
            stmt = stmt->next;
        }
    }
    
    // Validate return statement exists
    if (!module->data.module.return_stmt) {
        semantic_error(tc, module->line_number,
                      "Module '%s' missing return statement",
                      module->data.module.name);
    }
    
    // Return to parent scope
    tc->current_scope = module_scope->parent;
}

void analyze_statement(TypeChecker* tc, ASTNode* stmt) {
    if (!stmt) return;
    
    switch (stmt->type) {
        case NODE_ASSIGNMENT: {
            char* target = stmt->data.assignment.target;
            ASTNode* value = stmt->data.assignment.value;
            ASTNode* from_source = stmt->data.assignment.from_source;
            
            // Check if target already exists (redefinition)
            Symbol* existing = scope_lookup(tc->current_scope, target, 0);
            if (existing) {
                semantic_warning(tc, stmt->line_number,
                               "Redefining variable '%s'", target);
            }
            
            // Validate 'from' source exists
            TensorType* input_type = NULL;
            if (from_source) {
                if (from_source->type == NODE_IDENTIFIER) {
                    Symbol* src_sym = scope_lookup(tc->current_scope, 
                                                   from_source->data.identifier.name, 1);
                    if (!src_sym) {
                        semantic_error(tc, stmt->line_number,
                                     "Undefined variable '%s'",
                                     from_source->data.identifier.name);
                        break;
                    }
                    input_type = src_sym->tensor_type;
                }
            }
            
            // Infer output shape based on layer type
            TensorType* output_type = NULL;
            switch (value->type) {
                case NODE_CONV2D:
                    output_type = infer_conv2d_shape(tc, value, input_type);
                    break;
                case NODE_DENSE:
                    output_type = infer_dense_shape(tc, value, input_type);
                    break;
                case NODE_MAXPOOL:
                case NODE_AVGPOOL:
                    output_type = infer_pool_shape(tc, value, input_type);
                    break;
                case NODE_FLATTEN:
                    output_type = infer_flatten_shape(input_type);
                    break;
                case NODE_CONCAT:
                    output_type = infer_concat_shape(tc, value->data.concat.inputs);
                    break;
                case NODE_MODULE_CALL: {
                    // Look up module definition
                    Symbol* mod_sym = scope_lookup(tc->current_scope, 
                                                   value->data.module_call.module_name, 1);
                    if (!mod_sym || mod_sym->sym_type != SYM_MODULE) {
                        semantic_error(tc, stmt->line_number,
                                     "Undefined module '%s'",
                                     value->data.module_call.module_name);
                    }
                    // TODO: Validate arguments match parameters
                    break;
                }
                default:
                    semantic_warning(tc, stmt->line_number,
                                   "Unknown layer type in assignment");
                    break;
            }
            
            // Add variable to symbol table with inferred type
            Symbol* var_sym = scope_insert(tc->current_scope, target,
                                          SYM_VARIABLE, stmt, stmt->line_number);
            var_sym->tensor_type = output_type;
            
            break;
        }
        
        default:
            semantic_warning(tc, stmt->line_number,
                           "Unhandled statement type");
            break;
    }
}

void analyze_expression(TypeChecker* tc, ASTNode* expr) {
    // TODO: Implement expression type checking
    // For now, expressions are validated during statement analysis
}