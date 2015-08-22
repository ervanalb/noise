#include <stdlib.h>
#include "error.h"
#include "block.h"

struct state {
    object_t * output;
};

static error_t accumulator_pull(node_t * node, object_t ** output)
{
    struct state * state = (struct state *) &node->state->object_data;

    object_t * input0;
    pull(node, 0, &input0);

    CAST_OBJECT(double, state->output) += CAST_OBJECT(double, input0);

    *output = state->output;

    return SUCCESS;
}

#define N_INPUTS 1
#define N_OUTPUTS 1

node_t * accumulator_create()
{
    static type_t * state_type = NULL;

    if (state_type == NULL)
        state_type = make_simple_type(sizeof(struct state));

    if (state_type == NULL) return NULL;

    node_t * node = calloc(1, sizeof(node_t) + N_OUTPUTS * sizeof(struct endpoint));
    if (node == NULL) return NULL;

    node->n_inputs = N_INPUTS;
    node->inputs = calloc(N_INPUTS, sizeof(struct endpoint *));
    if (node->inputs == NULL) return (free(node), NULL);

    node->n_outputs = N_OUTPUTS;
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &accumulator_pull,
        .type = &double_type,
        .name = "delta",
    };

    node->state = object_alloc(state_type);
    if (node->state == NULL) return (free(node->inputs), free(node), NULL);

    struct state * state = (struct state *) &node->state->object_data;
    state->output = object_alloc(&double_type);
    CAST_OBJECT(double, state->output) = 0.0;

    node->destroy = &generic_block_destroy;
    return node;
}

