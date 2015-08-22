#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "typefns.h"

node_t * allocate_node(size_t n_inputs, size_t n_outputs, const type_t * state_type)
{
    node_t * node = calloc(1, sizeof(node_t) + n_outputs * sizeof(struct endpoint));
    if (node == NULL) return NULL;

    // Set up inputs array
    node->n_inputs = n_inputs;
    node->inputs = calloc(n_inputs, sizeof(struct node_input));
    if (node->inputs == NULL) goto fail;

    // Set up outputs array
    node->n_outputs = n_outputs;

    // Allocate state
    node->state = object_alloc(state_type);
    if (node->state == NULL) goto fail;

    return node;

fail:
    object_free(node->state);
    free(node->inputs);
    free(node);
    return NULL;
}


void generic_block_destroy(node_t * node)
{
    object_free(node->state);
    free(node->inputs);
    free(node);
}

error_t connect(struct node * dst, size_t dst_idx, struct node * src, size_t src_idx)
{
    if (src_idx >= src->n_outputs) {
        printf("connect error: source index too large (%lu >= %lu)\n", src_idx, src->n_outputs);
        return ERR_INVALID;
    } 

    if (dst_idx >= dst->n_inputs) {
        printf("connect error: dest index too large (%lu >= %lu)\n", dst_idx, dst->n_inputs);
        return ERR_INVALID;
    } 

    if (src->outputs[src_idx].type != dst->inputs[dst_idx].type) {
        printf("connect error: type mismatch %p %p\n", src->outputs[src_idx].type, dst->inputs[dst_idx].type);
    }

    dst->inputs[dst_idx].connected_input = &src->outputs[src_idx];

    //printf("connect success\n");
    return SUCCESS;
}

