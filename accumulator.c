#include <stdlib.h>
#include "error.h"
#include "block.h"

static error_t accumulator_pull(node_t * node, object_t ** output)
{
    object_t * input0;
    pull(node, 0, &input0);

    CAST_OBJECT(double, node->state) += CAST_OBJECT(double, input0);

    *output = node->state;
    return SUCCESS;
}

#define N_INPUTS 1
#define N_OUTPUTS 1

node_t * allocate_node(size_t n_inputs, size_t n_outputs, type_t * state_type)
{
    node_t * node = calloc(1, sizeof(node_t) + N_OUTPUTS * sizeof(struct endpoint));
    if (node == NULL) return NULL;

    // Set up inputs array
    node->n_inputs = n_inputs;
    node->inputs = calloc(n_inputs, sizeof(struct endpoint *));
    if (node->inputs == NULL) return (free(node), NULL);

    // Set up outputs array
    node->n_outputs = n_outputs;

    // Allocate state
    node->state = object_alloc(state_type);
    if (node->state == NULL) return (free(node->inputs), free(node), NULL);

    return node;
}

node_t * accumulator_create()
{
    node_t * node = allocate_node(1, 1, double_type);
    node->destroy = &generic_block_destroy;
    
    // Define outputs (0: double sum)
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &accumulator_pull,
        .type = double_type,
        .name = "sum",
    };

    // Initialize state
    
    CAST_OBJECT(double, node->state) = 0.0;

    return node;
}

