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

node_t * accumulator_create()
{
    // Allocate new node_t 
    
    node_t * node = calloc(1, sizeof(node_t) + N_OUTPUTS * sizeof(struct endpoint));
    if (node == NULL) return NULL;

    // Set up inputs array

    node->n_inputs = N_INPUTS;
    node->inputs = calloc(N_INPUTS, sizeof(struct endpoint *));
    if (node->inputs == NULL) return (free(node), NULL);

    // Define outputs (0: double sum)

    node->n_outputs = N_OUTPUTS;
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &accumulator_pull,
        .type = &double_type,
        .name = "sum",
    };

    // Initialize state

    node->state = object_alloc(&double_type);
    if (node->state == NULL) return (free(node->inputs), free(node), NULL);
    CAST_OBJECT(double, node->state) = 0.0;

    node->destroy = &generic_block_destroy;
    return node;
}

