#include <stdlib.h>
#include "block.h"

#define N_OUTPUTS 1

static error_t constant_pull(node_t * node, object_t ** output)
{
	*output = node->state;
	return SUCCESS;
}

node_t * constant_create(object_t * constant_value)
{
    node_t * node = calloc(1, sizeof(node_t) + N_OUTPUTS * sizeof(struct endpoint));
    if (node == NULL) return NULL;

    node->n_inputs = 0;
    node->inputs = NULL;

    node->n_outputs = N_OUTPUTS;
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &constant_pull,
        .type = constant_value->object_type,
        .name = "constant",
    };

    node->state = constant_value;

    node->destroy = &generic_block_destroy;

    return node;
}

