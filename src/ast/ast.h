/*
 * NetLang AST - Abstract Syntax Tree Definitions
 * 
 * Defines all node types for representing NetLang programs.
 * The AST is built by the parser and consumed by semantic analysis.
 * 
 * Author: Abhijeet Deb Nath
 * Course: Compiler Design
 */

#ifndef NETLANG_AST_H
#define NETLANG_AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== FORWARD DECLARATIONS ========== */
typedef struct ASTNode ASTNode;
typedef struct ASTList ASTList;
typedef struct Parameter Parameter;
typedef struct ParameterList ParameterList;
typedef struct FusionInfo FusionInfo;
typedef struct BlockingInfo BlockingInfo;

/* ========== NODE TYPES ========== */
typedef enum {
    /* Top-level constructs */
    NODE_PROGRAM,           /* Root node containing all networks/modules */
    NODE_NETWORK,           /* network Name { ... } */
    NODE_MODULE,            /* module Name(params) { ... } */
    
    /* Network components */
    NODE_INPUT,             /* input(shape: [...]) */
    NODE_WEIGHTS,           /* weights("path") */
    NODE_STATEMENT_LIST,    /* List of statements in a block */
    
    /* Statements */
    NODE_ASSIGNMENT,        /* x = Layer(...) from source */
    NODE_RETURN,            /* return Concat(...) or return expr */
    
    /* Layer types */
    NODE_CONV2D,            /* Conv2D(filters:, kernel:, ...) */
    NODE_DENSE,             /* Dense(units:, activation:) */
    NODE_MAXPOOL,           /* MaxPool(pool:, stride:) */
    NODE_AVGPOOL,           /* AvgPool(pool:, stride:) */
    NODE_FLATTEN,           /* Flatten() */
    NODE_ADD,               /* Add(x, y, ...) */
    NODE_CONCAT,            /* Concat(b1, b2, ...) */
    NODE_BATCHNORM,         /* BatchNorm() */
    NODE_LAYERNORM,         /* LayerNorm() */
    
    /* Module instantiation */
    NODE_MODULE_CALL,       /* InceptionBlock(filters_1x1: 64, ...) */
    
    /* Expressions */
    NODE_IDENTIFIER,        /* Variable reference (x, input) */
    NODE_NUMBER,            /* Integer or float literal */
    NODE_STRING,            /* String literal ("path/to/file") */
    NODE_ARRAY,             /* Array literal [28, 28, 1] */
    
    /* Special */
    NODE_FROM_EXPR,         /* Wrapper for "Layer(...) from source" */
    
} NodeType;

/* ========== ACTIVATION TYPES ========== */
typedef enum {
    ACT_NONE,
    ACT_RELU,
    ACT_SIGMOID,
    ACT_TANH,
    ACT_SOFTMAX,
    ACT_LINEAR
} ActivationType;

/* ========== PARAMETER STRUCTURE ========== */
/* For named parameters like: filters: 32, kernel: [3, 3] */
struct Parameter {
    char* name;             /* Parameter name (filters, kernel, etc.) */
    ASTNode* value;         /* Parameter value (can be number, array, identifier) */
    Parameter* next;        /* Next parameter in list */
};

struct ParameterList {
    Parameter* head;
    Parameter* tail;
    int count;
};

/* ========== NETWORK BODY COLLECTOR ========== */
/* Used by parser to collect network components before building AST node */
typedef struct NetworkBody {
    ASTNode* input;
    ASTNode* weights;
    ASTList* statements;
} NetworkBody;

/* ========== LIST STRUCTURE ========== */
/* Generic list of AST nodes */
struct ASTList {
    ASTNode* head;
    ASTNode* tail;
    int count;
};

/* ========== MAIN AST NODE ========== */
struct ASTNode {
    NodeType type;
    int line_number;        /* Source line for error reporting */
    
    union {
        /* NODE_PROGRAM */
        struct {
            ASTList* definitions;   /* List of networks and modules */
        } program;
        
        /* NODE_NETWORK */
        struct {
            char* name;             /* Network name (MNIST_CNN, etc.) */
            ASTNode* input;         /* Input declaration */
            ASTNode* weights;       /* Weights path (optional) */
            ASTList* statements;    /* Layer assignments */
        } network;
        
        /* NODE_MODULE */
        struct {
            char* name;             /* Module name (InceptionBlock, etc.) */
            ParameterList* params;  /* Module parameters */
            ASTList* statements;    /* Module body */
            ASTNode* return_stmt;   /* Return statement */
        } module;
        
        /* NODE_INPUT */
        struct {
            ASTNode* shape;         /* Shape array [28, 28, 1] */
        } input;
        
        /* NODE_WEIGHTS */
        struct {
            char* path;             /* File path string */
        } weights;
        
        /* NODE_ASSIGNMENT */
        struct {
            char* target;           /* Variable name (x, b1, etc.) */
            ASTNode* value;         /* Layer/expression being assigned */
            ASTNode* from_source;   /* Source tensor (after 'from') */
        } assignment;
        
        /* NODE_RETURN */
        struct {
            ASTNode* value;         /* Return expression */
        } return_stmt;
        
        /* NODE_CONV2D */
        struct {
            int filters;
            int kernel[2];          /* [height, width] */
            int stride;
            int padding;
            ActivationType activation;
        } conv2d;
        
        /* NODE_DENSE */
        struct {
            int units;
            ActivationType activation;
        } dense;
        
        /* NODE_MAXPOOL, NODE_AVGPOOL */
        struct {
            int pool[2];            /* [height, width] */
            int stride;
            int padding;
        } pooling;
        
        /* NODE_ADD */
        struct {
            ASTList* inputs;        /* List of tensors to add elementwise */
        } add;

        /* NODE_CONCAT */
        struct {
            ASTList* inputs;        /* List of tensors to concatenate */
        } concat;
        
        /* NODE_MODULE_CALL */
        struct {
            char* module_name;      /* Name of module being instantiated */
            ParameterList* args;    /* Arguments passed to module */
        } module_call;
        
        /* NODE_IDENTIFIER */
        struct {
            char* name;
        } identifier;
        
        /* NODE_NUMBER */
        struct {
            int is_float;
            union {
                int ival;
                float fval;
            };
        } number;
        
        /* NODE_STRING */
        struct {
            char* value;
        } string;
        
        /* NODE_ARRAY */
        struct {
            ASTList* elements;      /* List of number nodes */
        } array;
        
        /* NODE_FROM_EXPR */
        struct {
            ASTNode* layer;         /* The layer expression */
            ASTNode* source;        /* The source tensor */
        } from_expr;
        
    } data;
    
    /* ========== OPTIMIZATION METADATA ========== */
    /* Added by optimization passes, used during code generation */
    FusionInfo* fusion_info;        /* Operator fusion metadata */
    BlockingInfo* blocking_info;    /* Cache blocking configuration */
    
    /* For linked lists */
    ASTNode* next;
};

/* ========== CONSTRUCTOR FUNCTIONS ========== */

/* Memory allocation with error checking */
static inline void* ast_alloc(size_t size) {
    void* ptr = calloc(1, size);
    if (!ptr) {
        fprintf(stderr, "FATAL: Memory allocation failed\n");
        exit(1);
    }
    return ptr;
}

/* Create a new AST node */
static inline ASTNode* ast_node_new(NodeType type, int line) {
    ASTNode* node = (ASTNode*)ast_alloc(sizeof(ASTNode));
    node->type = type;
    node->line_number = line;
    node->next = NULL;
    node->fusion_info = NULL;
    node->blocking_info = NULL;
    return node;
}

/* Create a new AST list */
static inline ASTList* ast_list_new(void) {
    ASTList* list = (ASTList*)ast_alloc(sizeof(ASTList));
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    return list;
}

/* Append node to list */
static inline void ast_list_append(ASTList* list, ASTNode* node) {
    if (!list || !node) return;
    if (!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->count++;
}

/* Create parameter list */
static inline ParameterList* param_list_new(void) {
    ParameterList* list = (ParameterList*)ast_alloc(sizeof(ParameterList));
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    return list;
}

/* Create and add parameter */
static inline void param_list_add(ParameterList* list, const char* name, ASTNode* value) {
    if (!list) return;
    Parameter* param = (Parameter*)ast_alloc(sizeof(Parameter));
    param->name = strdup(name);
    param->value = value;
    param->next = NULL;
    
    if (!list->head) {
        list->head = param;
        list->tail = param;
    } else {
        list->tail->next = param;
        list->tail = param;
    }
    list->count++;
}

/* ========== CONVENIENCE CONSTRUCTORS ========== */

/* Create program node (root) */
static inline ASTNode* ast_program(int line) {
    ASTNode* node = ast_node_new(NODE_PROGRAM, line);
    node->data.program.definitions = ast_list_new();
    return node;
}

/* Create network node */
static inline ASTNode* ast_network(const char* name, int line) {
    ASTNode* node = ast_node_new(NODE_NETWORK, line);
    node->data.network.name = strdup(name);
    node->data.network.input = NULL;
    node->data.network.weights = NULL;
    node->data.network.statements = ast_list_new();
    return node;
}

/* Create module node */
static inline ASTNode* ast_module(const char* name, ParameterList* params, int line) {
    ASTNode* node = ast_node_new(NODE_MODULE, line);
    node->data.module.name = strdup(name);
    node->data.module.params = params;
    node->data.module.statements = ast_list_new();
    node->data.module.return_stmt = NULL;
    return node;
}

/* Create identifier node */
static inline ASTNode* ast_identifier(const char* name, int line) {
    ASTNode* node = ast_node_new(NODE_IDENTIFIER, line);
    node->data.identifier.name = strdup(name);
    return node;
}

/* Create integer number node */
static inline ASTNode* ast_number_int(int value, int line) {
    ASTNode* node = ast_node_new(NODE_NUMBER, line);
    node->data.number.is_float = 0;
    node->data.number.ival = value;
    return node;
}

/* Create float number node */
static inline ASTNode* ast_number_float(float value, int line) {
    ASTNode* node = ast_node_new(NODE_NUMBER, line);
    node->data.number.is_float = 1;
    node->data.number.fval = value;
    return node;
}

/* Create string node */
static inline ASTNode* ast_string(const char* value, int line) {
    ASTNode* node = ast_node_new(NODE_STRING, line);
    node->data.string.value = strdup(value);
    return node;
}

/* Create array node */
static inline ASTNode* ast_array(int line) {
    ASTNode* node = ast_node_new(NODE_ARRAY, line);
    node->data.array.elements = ast_list_new();
    return node;
}

/* Create assignment node */
static inline ASTNode* ast_assignment(const char* target, ASTNode* value, ASTNode* from, int line) {
    ASTNode* node = ast_node_new(NODE_ASSIGNMENT, line);
    node->data.assignment.target = strdup(target);
    node->data.assignment.value = value;
    node->data.assignment.from_source = from;
    return node;
}

/* Create input node */
static inline ASTNode* ast_input(ASTNode* shape, int line) {
    ASTNode* node = ast_node_new(NODE_INPUT, line);
    node->data.input.shape = shape;
    return node;
}

/* Create weights node */
static inline ASTNode* ast_weights(const char* path, int line) {
    ASTNode* node = ast_node_new(NODE_WEIGHTS, line);
    node->data.weights.path = strdup(path);
    return node;
}

/* Convert activation string to enum */
static inline ActivationType activation_from_string(const char* name) {
    if (!name) return ACT_NONE;
    if (strcmp(name, "relu") == 0) return ACT_RELU;
    if (strcmp(name, "sigmoid") == 0) return ACT_SIGMOID;
    if (strcmp(name, "tanh") == 0) return ACT_TANH;
    if (strcmp(name, "softmax") == 0) return ACT_SOFTMAX;
    if (strcmp(name, "linear") == 0) return ACT_LINEAR;
    return ACT_NONE;
}

/* Get activation name string */
static inline const char* activation_to_string(ActivationType act) {
    switch (act) {
        case ACT_RELU: return "relu";
        case ACT_SIGMOID: return "sigmoid";
        case ACT_TANH: return "tanh";
        case ACT_SOFTMAX: return "softmax";
        case ACT_LINEAR: return "linear";
        default: return "none";
    }
}

/* ========== DEBUG PRINTING ========== */

void ast_print(ASTNode* node, int indent);
void ast_free(ASTNode* node);

#endif /* NETLANG_AST_H */
