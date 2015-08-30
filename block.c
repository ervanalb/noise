#include <string.h>
#include <stdlib.h>

#include "block.h"
#include "error.h"
#include "ntypes.h"
#include "util.h"

int node_alloc_connections(node_t * node, size_t n_inputs, size_t n_outputs) {
    assert(node);

    // Set up inputs array
    node->node_n_inputs = n_inputs;
    node->node_inputs = calloc(n_inputs, sizeof(*node->node_inputs));
    if (node->node_inputs == NULL) goto fail;

    // Set up outputs array
    node->node_n_outputs = n_outputs;
    node->node_outputs = calloc(n_outputs, sizeof(*node->node_outputs));
    if (node->node_outputs == NULL) goto fail;

    return 0;
fail:
    free(node->node_inputs);
    free(node->node_outputs);
    return (errno = ENOMEM, -errno);
}

void node_free_connections(node_t * node) {
    for (size_t i = 0; i < node->node_n_inputs; i++) {
        free(node->node_inputs[i].inport_name);
    }

    for (size_t i = 0; i < node->node_n_outputs; i++) {
        free(node->node_outputs[i].port_name); 
    }

    free(node->node_inputs);
    free(node->node_name);
}

void node_term_generic(node_t * node) {
    node_free_connections(node);
    free(node->node_state);
}

void node_term_generic_objstate(node_t * node) {
    node_free_connections(node);
    object_free(node->node_state);
}

// Return a copy of the node, *and* any input connections
// If flags.can_copy is set, node->node_state must be NULL or an object_t
node_t * node_dup(const node_t * src) {
    if (src == NULL || !src->node_flags.flag_can_copy) return (errno = EINVAL, NULL);

    node_t * dst = calloc(1, sizeof(*dst));
    if (dst == NULL) return (errno = ENOMEM, NULL);

    int rc = node_alloc_connections(dst, src->node_n_inputs, src->node_n_outputs);
    if (rc != 0) goto fail;

    dst->node_name = src->node_name;
    dst->node_term = src->node_term;

    // Copy inputs & outputs
    memcpy(dst->node_inputs, src->node_inputs, sizeof(*src->node_inputs) * src->node_n_inputs);
    memcpy(dst->node_outputs, src->node_outputs, sizeof(*src->node_outputs) * src->node_n_outputs);

    // Copy state
    if (src->node_state != NULL) {
        dst->node_state = object_dup(src->node_state);
        if (dst->node_state == NULL) goto fail;
    }

    return dst;

fail:
    free(dst->node_inputs);
    free(dst->node_outputs);
    object_free(dst->node_state);
    free(dst);
    return NULL;
}

// 

int port_connect(struct port * output, struct inport * input) {
    if (!type_compatible(output->port_type, input->inport_type))
        return (errno = EINVAL, -errno);

    input->inport_connection = output;
    return 0;
}

int node_connect(node_t * output, size_t out_idx, node_t * input, size_t in_idx){
    return port_connect(&output->node_outputs[out_idx], &input->node_inputs[in_idx]);
}
