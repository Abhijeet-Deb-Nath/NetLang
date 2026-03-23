#ifndef NETLANG_GRAPH_H
#define NETLANG_GRAPH_H

#include "../ast/ast.h"
#include "../semantic/symbol_table.h"
#include <stdio.h>

typedef struct GraphValue GraphValue;
typedef struct GraphNode GraphNode;
typedef struct NetGraph NetGraph;

struct GraphValue {
    int id;
    char* name;              /* Source-level name at definition time */
    char* storage_name;      /* Unique storage name for generated code */
    TensorType* type;
    GraphNode* producer;
    GraphNode** consumers;
    int consumer_count;
    int consumer_capacity;
};

struct GraphNode {
    int id;
    NodeType op_type;
    ASTNode* ast_node;
    GraphValue** inputs;
    int input_count;
    GraphValue* output;
    int topo_index;
};

struct NetGraph {
    char* network_name;
    char* weight_path;
    TensorType* input_type;
    GraphValue* input_value;
    GraphValue* output_value;

    GraphNode** nodes;
    int node_count;
    int node_capacity;

    GraphValue** values;
    int value_count;
    int value_capacity;

    GraphNode** topo_nodes;
    int topo_count;
};

NetGraph* netgraph_build(ASTNode* network, FILE* error_stream);
void netgraph_free(NetGraph* graph);
const char* netgraph_node_type_name(NodeType type);

#endif /* NETLANG_GRAPH_H */
