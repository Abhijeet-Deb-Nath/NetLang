#include "graph.h"
#include "../semantic/type_checker.h"
#include <stdarg.h>

typedef struct Binding {
    char* name;
    GraphValue* value;
} Binding;

typedef struct BindingTable {
    Binding* items;
    int count;
    int capacity;
} BindingTable;

static void graph_error(FILE* stream, int line, const char* fmt, ...) {
    va_list args;

    fprintf(stream ? stream : stderr, "Graph error");
    if (line > 0) {
        fprintf(stream ? stream : stderr, " (line %d)", line);
    }
    fprintf(stream ? stream : stderr, ": ");

    va_start(args, fmt);
    vfprintf(stream ? stream : stderr, fmt, args);
    va_end(args);

    fprintf(stream ? stream : stderr, "\n");
}

static int ensure_node_capacity(NetGraph* graph) {
    if (graph->node_count < graph->node_capacity) {
        return 1;
    }

    int new_capacity = graph->node_capacity == 0 ? 8 : graph->node_capacity * 2;
    GraphNode** nodes = realloc(graph->nodes, new_capacity * sizeof(GraphNode*));
    if (!nodes) {
        return 0;
    }

    graph->nodes = nodes;
    graph->node_capacity = new_capacity;
    return 1;
}

static int ensure_value_capacity(NetGraph* graph) {
    if (graph->value_count < graph->value_capacity) {
        return 1;
    }

    int new_capacity = graph->value_capacity == 0 ? 8 : graph->value_capacity * 2;
    GraphValue** values = realloc(graph->values, new_capacity * sizeof(GraphValue*));
    if (!values) {
        return 0;
    }

    graph->values = values;
    graph->value_capacity = new_capacity;
    return 1;
}

static int ensure_binding_capacity(BindingTable* table) {
    if (table->count < table->capacity) {
        return 1;
    }

    int new_capacity = table->capacity == 0 ? 8 : table->capacity * 2;
    Binding* items = realloc(table->items, new_capacity * sizeof(Binding));
    if (!items) {
        return 0;
    }

    table->items = items;
    table->capacity = new_capacity;
    return 1;
}

static int graph_value_add_consumer(GraphValue* value, GraphNode* consumer) {
    if (value->consumer_count >= value->consumer_capacity) {
        int new_capacity = value->consumer_capacity == 0 ? 4 : value->consumer_capacity * 2;
        GraphNode** consumers = realloc(value->consumers, new_capacity * sizeof(GraphNode*));
        if (!consumers) {
            return 0;
        }
        value->consumers = consumers;
        value->consumer_capacity = new_capacity;
    }

    value->consumers[value->consumer_count++] = consumer;
    return 1;
}

static GraphValue* binding_lookup(BindingTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            return table->items[i].value;
        }
    }
    return NULL;
}

static int binding_set(BindingTable* table, const char* name, GraphValue* value) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            table->items[i].value = value;
            return 1;
        }
    }

    if (!ensure_binding_capacity(table)) {
        return 0;
    }

    table->items[table->count].name = strdup(name);
    table->items[table->count].value = value;
    table->count++;
    return 1;
}

static void binding_table_free(BindingTable* table) {
    for (int i = 0; i < table->count; i++) {
        free(table->items[i].name);
    }
    free(table->items);
}

static TensorType* input_type_from_ast(ASTNode* input_node, FILE* error_stream) {
    if (!input_node || input_node->type != NODE_INPUT) {
        graph_error(error_stream, input_node ? input_node->line_number : 0,
                    "network is missing a valid input declaration");
        return NULL;
    }

    ASTNode* shape_array = input_node->data.input.shape;
    if (!shape_array || shape_array->type != NODE_ARRAY || !shape_array->data.array.elements) {
        graph_error(error_stream, input_node->line_number,
                    "input shape must be an array literal");
        return NULL;
    }

    if (shape_array->data.array.elements->count != 3) {
        graph_error(error_stream, input_node->line_number,
                    "input shape must be [H, W, C]");
        return NULL;
    }

    int dims[3] = {0, 0, 0};
    int idx = 0;
    ASTNode* elem = shape_array->data.array.elements->head;

    while (elem && idx < 3) {
        if (elem->type != NODE_NUMBER || elem->data.number.is_float) {
            graph_error(error_stream, elem->line_number,
                        "input shape dimensions must be integers");
            return NULL;
        }
        if (elem->data.number.ival <= 0) {
            graph_error(error_stream, elem->line_number,
                        "input shape dimensions must be positive");
            return NULL;
        }
        dims[idx++] = elem->data.number.ival;
        elem = elem->next;
    }

    return tensor_type_create(3, dims);
}

static TensorType* infer_output_type(ASTNode* layer_node,
                                     GraphValue** inputs,
                                     int input_count,
                                     FILE* error_stream) {
    TensorType* output = NULL;
    const TensorType* input = input_count > 0 ? inputs[0]->type : NULL;

    switch (layer_node->type) {
        case NODE_CONV2D: {
            if (!input || input->rank != 3) {
                graph_error(error_stream, layer_node->line_number,
                            "Conv2D requires 3D input");
                return NULL;
            }

            output = tensor_type_clone(input);
            output->dims[0] = (input->dims[0] + 2 * layer_node->data.conv2d.padding
                             - layer_node->data.conv2d.kernel[0])
                            / layer_node->data.conv2d.stride + 1;
            output->dims[1] = (input->dims[1] + 2 * layer_node->data.conv2d.padding
                             - layer_node->data.conv2d.kernel[1])
                            / layer_node->data.conv2d.stride + 1;
            output->dims[2] = layer_node->data.conv2d.filters;
            return output;
        }

        case NODE_DENSE: {
            int dims[1] = {layer_node->data.dense.units};
            return tensor_type_create(1, dims);
        }

        case NODE_MAXPOOL:
        case NODE_AVGPOOL: {
            int stride = layer_node->data.pooling.stride;

            if (!input || input->rank != 3) {
                graph_error(error_stream, layer_node->line_number,
                            "Pooling requires 3D input");
                return NULL;
            }

            if (stride == 0) {
                stride = layer_node->data.pooling.pool[0];
            }

            output = tensor_type_clone(input);
            output->dims[0] = (input->dims[0] + 2 * layer_node->data.pooling.padding
                             - layer_node->data.pooling.pool[0]) / stride + 1;
            output->dims[1] = (input->dims[1] + 2 * layer_node->data.pooling.padding
                             - layer_node->data.pooling.pool[1]) / stride + 1;
            return output;
        }

        case NODE_FLATTEN: {
            int features = 1;
            int dims[1];

            if (!input) {
                graph_error(error_stream, layer_node->line_number,
                            "Flatten requires an input value");
                return NULL;
            }

            if (input->rank == 3) {
                features = input->dims[0] * input->dims[1] * input->dims[2];
            } else if (input->rank == 1) {
                features = input->dims[0];
            } else {
                graph_error(error_stream, layer_node->line_number,
                            "Flatten only supports 1D or 3D tensors");
                return NULL;
            }

            dims[0] = features;
            return tensor_type_create(1, dims);
        }

        case NODE_ADD: {
            if (input_count == 0) {
                graph_error(error_stream, layer_node->line_number,
                            "Add requires at least one input");
                return NULL;
            }

            TensorType* base_type = inputs[0]->type;
            if (!base_type) {
                graph_error(error_stream, layer_node->line_number,
                            "Add input is missing type information");
                return NULL;
            }

            for (int i = 1; i < input_count; i++) {
                TensorType* value_type = inputs[i]->type;
                if (!value_type || value_type->rank != base_type->rank) {
                    graph_error(error_stream, layer_node->line_number,
                                "Add inputs must have matching tensor ranks");
                    return NULL;
                }

                for (int dim = 0; dim < base_type->rank; dim++) {
                    if (value_type->dims[dim] != base_type->dims[dim]) {
                        graph_error(error_stream, layer_node->line_number,
                                    "Add inputs must have identical shapes");
                        return NULL;
                    }
                }
            }

            return tensor_type_clone(base_type);
        }

        case NODE_CONCAT: {
            if (input_count == 0) {
                graph_error(error_stream, layer_node->line_number,
                            "Concat requires at least one input");
                return NULL;
            }

            int total_channels = 0;
            int dims[3];

            for (int i = 0; i < input_count; i++) {
                TensorType* value_type = inputs[i]->type;

                if (!value_type || value_type->rank != 3) {
                    graph_error(error_stream, layer_node->line_number,
                                "Concat currently requires 3D inputs");
                    return NULL;
                }

                if (i == 0) {
                    dims[0] = value_type->dims[0];
                    dims[1] = value_type->dims[1];
                    dims[2] = 0;
                } else if (value_type->dims[0] != dims[0] || value_type->dims[1] != dims[1]) {
                    graph_error(error_stream, layer_node->line_number,
                                "Concat inputs must have matching H and W dimensions");
                    return NULL;
                }

                total_channels += value_type->dims[2];
            }

            dims[2] = total_channels;
            return tensor_type_create(3, dims);
        }

        default:
            graph_error(error_stream, layer_node->line_number,
                        "unsupported layer type '%s' in graph lowering",
                        netgraph_node_type_name(layer_node->type));
            return NULL;
    }
}

static GraphValue* create_value(NetGraph* graph,
                                const char* name,
                                TensorType* type,
                                GraphNode* producer) {
    GraphValue* value = calloc(1, sizeof(GraphValue));
    if (!value) {
        return NULL;
    }

    value->id = graph->value_count;
    value->name = strdup(name);
    value->type = type;
    value->producer = producer;

    char storage_name[32];
    snprintf(storage_name, sizeof(storage_name), "value_%d", value->id);
    value->storage_name = strdup(storage_name);

    if (!ensure_value_capacity(graph)) {
        free(value->storage_name);
        free(value->name);
        free(value);
        return NULL;
    }

    graph->values[graph->value_count++] = value;
    return value;
}

static GraphNode* create_node(NetGraph* graph,
                              ASTNode* layer_node,
                              GraphValue** inputs,
                              int input_count) {
    GraphNode* node = calloc(1, sizeof(GraphNode));
    if (!node) {
        return NULL;
    }

    node->id = graph->node_count;
    node->op_type = layer_node->type;
    node->ast_node = layer_node;
    node->input_count = input_count;
    node->topo_index = -1;

    if (input_count > 0) {
        node->inputs = calloc((size_t)input_count, sizeof(GraphValue*));
        if (!node->inputs) {
            free(node);
            return NULL;
        }
        memcpy(node->inputs, inputs, (size_t)input_count * sizeof(GraphValue*));
    }

    if (!ensure_node_capacity(graph)) {
        free(node->inputs);
        free(node);
        return NULL;
    }

    graph->nodes[graph->node_count++] = node;
    return node;
}

static int build_topological_order(NetGraph* graph, FILE* error_stream) {
    int* indegree = calloc((size_t)graph->node_count, sizeof(int));
    GraphNode** queue = calloc((size_t)graph->node_count, sizeof(GraphNode*));
    GraphNode** topo = calloc((size_t)graph->node_count, sizeof(GraphNode*));
    int head = 0;
    int tail = 0;
    int topo_count = 0;

    if (!indegree || !queue || !topo) {
        free(indegree);
        free(queue);
        free(topo);
        return 0;
    }

    for (int i = 0; i < graph->node_count; i++) {
        GraphNode* node = graph->nodes[i];
        for (int j = 0; j < node->input_count; j++) {
            if (node->inputs[j] && node->inputs[j]->producer) {
                indegree[i]++;
            }
        }
        if (indegree[i] == 0) {
            queue[tail++] = node;
        }
    }

    while (head < tail) {
        GraphNode* node = queue[head++];
        topo[topo_count] = node;
        node->topo_index = topo_count++;

        if (!node->output) {
            continue;
        }

        for (int i = 0; i < node->output->consumer_count; i++) {
            GraphNode* consumer = node->output->consumers[i];
            indegree[consumer->id]--;
            if (indegree[consumer->id] == 0) {
                queue[tail++] = consumer;
            }
        }
    }

    free(indegree);
    free(queue);

    if (topo_count != graph->node_count) {
        graph_error(error_stream, 0, "graph contains a cycle or unresolved dependency");
        free(topo);
        return 0;
    }

    graph->topo_nodes = topo;
    graph->topo_count = topo_count;
    return 1;
}

NetGraph* netgraph_build(ASTNode* network, FILE* error_stream) {
    BindingTable bindings = {0};
    NetGraph* graph = NULL;
    TensorType* input_type = NULL;
    GraphValue* input_value = NULL;
    GraphValue* last_output = NULL;
    ASTNode* stmt = NULL;

    if (!network || network->type != NODE_NETWORK) {
        graph_error(error_stream, network ? network->line_number : 0,
                    "expected a network node");
        return NULL;
    }

    graph = calloc(1, sizeof(NetGraph));
    if (!graph) {
        binding_table_free(&bindings);
        return NULL;
    }

    graph->network_name = strdup(network->data.network.name);
    graph->weight_path = network->data.network.weights
        ? strdup(network->data.network.weights->data.weights.path)
        : strdup("weights.nwf");

    input_type = input_type_from_ast(network->data.network.input, error_stream);
    if (!input_type) {
        goto fail;
    }

    graph->input_type = tensor_type_clone(input_type);
    if (!graph->input_type) {
        goto fail;
    }

    input_value = create_value(graph, "input", input_type, NULL);
    if (!input_value) {
        goto fail;
    }
    input_type = NULL;

    graph->input_value = input_value;
    if (!binding_set(&bindings, "input", input_value)) {
        goto fail;
    }

    stmt = network->data.network.statements ? network->data.network.statements->head : NULL;
    while (stmt) {
        ASTNode* layer_node = stmt->data.assignment.value;
        GraphValue* inputs[32];
        int input_count = 0;
        TensorType* output_type = NULL;
        GraphNode* node = NULL;
        GraphValue* output_value = NULL;

        if (!stmt || stmt->type != NODE_ASSIGNMENT) {
            graph_error(error_stream, stmt ? stmt->line_number : 0,
                        "graph lowering only supports assignment statements");
            goto fail;
        }

        if (!layer_node) {
            graph_error(error_stream, stmt->line_number, "assignment is missing a layer value");
            goto fail;
        }

        if (layer_node->type == NODE_ADD || layer_node->type == NODE_CONCAT) {
            ASTList* input_list = (layer_node->type == NODE_ADD)
                ? layer_node->data.add.inputs
                : layer_node->data.concat.inputs;
            ASTNode* input_expr = input_list ? input_list->head : NULL;

            while (input_expr) {
                if (input_expr->type != NODE_IDENTIFIER) {
                    graph_error(error_stream, input_expr->line_number,
                                "Concat inputs must be identifiers");
                    goto fail;
                }

                GraphValue* value = binding_lookup(&bindings, input_expr->data.identifier.name);
                if (!value) {
                    graph_error(error_stream, input_expr->line_number,
                                "undefined value '%s' in Concat",
                                input_expr->data.identifier.name);
                    goto fail;
                }

                if (input_count >= (int)(sizeof(inputs) / sizeof(inputs[0]))) {
                    graph_error(error_stream, input_expr->line_number,
                                "too many Concat inputs");
                    goto fail;
                }

                inputs[input_count++] = value;
                input_expr = input_expr->next;
            }
        } else {
            ASTNode* from_source = stmt->data.assignment.from_source;

            if (!from_source || from_source->type != NODE_IDENTIFIER) {
                graph_error(error_stream, stmt->line_number,
                            "layer inputs must be identifiers for graph lowering");
                goto fail;
            }

            GraphValue* value = binding_lookup(&bindings, from_source->data.identifier.name);
            if (!value) {
                graph_error(error_stream, stmt->line_number,
                            "undefined source value '%s'",
                            from_source->data.identifier.name);
                goto fail;
            }

            inputs[input_count++] = value;
        }

        output_type = infer_output_type(layer_node, inputs, input_count, error_stream);
        if (!output_type) {
            goto fail;
        }

        node = create_node(graph, layer_node, inputs, input_count);
        if (!node) {
            tensor_type_free(output_type);
            goto fail;
        }

        output_value = create_value(graph, stmt->data.assignment.target, output_type, node);
        if (!output_value) {
            tensor_type_free(output_type);
            goto fail;
        }

        node->output = output_value;

        for (int i = 0; i < input_count; i++) {
            if (!graph_value_add_consumer(inputs[i], node)) {
                goto fail;
            }
        }

        if (!binding_set(&bindings, stmt->data.assignment.target, output_value)) {
            goto fail;
        }

        last_output = output_value;
        stmt = stmt->next;
    }

    if (!last_output) {
        graph_error(error_stream, network->line_number,
                    "network must contain at least one assignment");
        goto fail;
    }

    graph->output_value = last_output;

    if (!build_topological_order(graph, error_stream)) {
        goto fail;
    }

    binding_table_free(&bindings);
    return graph;

fail:
    tensor_type_free(input_type);
    binding_table_free(&bindings);
    netgraph_free(graph);
    return NULL;
}

void netgraph_free(NetGraph* graph) {
    if (!graph) {
        return;
    }

    for (int i = 0; i < graph->node_count; i++) {
        free(graph->nodes[i]->inputs);
        free(graph->nodes[i]);
    }

    for (int i = 0; i < graph->value_count; i++) {
        GraphValue* value = graph->values[i];
        free(value->name);
        free(value->storage_name);
        tensor_type_free(value->type);
        free(value->consumers);
        free(value);
    }

    free(graph->nodes);
    free(graph->values);
    free(graph->topo_nodes);
    free(graph->network_name);
    free(graph->weight_path);
    tensor_type_free(graph->input_type);
    free(graph);
}

const char* netgraph_node_type_name(NodeType type) {
    switch (type) {
        case NODE_CONV2D: return "Conv2D";
        case NODE_DENSE: return "Dense";
        case NODE_MAXPOOL: return "MaxPool";
        case NODE_AVGPOOL: return "AvgPool";
        case NODE_FLATTEN: return "Flatten";
        case NODE_ADD: return "Add";
        case NODE_CONCAT: return "Concat";
        case NODE_BATCHNORM: return "BatchNorm";
        case NODE_LAYERNORM: return "LayerNorm";
        case NODE_MODULE_CALL: return "ModuleCall";
        default: return "Unknown";
    }
}
