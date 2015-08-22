#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "typefns.h"

node_t * allocate_node(size_t n_inputs, size_t n_outputs, type_t * state_type)
{
    node_t * node = calloc(1, sizeof(node_t) + n_outputs * sizeof(struct endpoint));
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


void generic_block_destroy(node_t * node)
{
    object_free(node->state);
}

error_t connect(struct node * dst, size_t dst_idx, struct node * src, size_t src_idx)
{
    if (src_idx >= src->n_outputs || dst_idx >= dst->n_inputs)
        return ERR_INVALID;

    dst->inputs[dst_idx] = &src->outputs[src_idx];
    return SUCCESS;
}

