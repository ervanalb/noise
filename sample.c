#include <stdlib.h>
#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "globals.h"
#include "util.h"
#include "math.h"

struct state {
    object_t * output;
	int t;
    int length;
    const double * sample;
};

static type_t * state_type;

static error_t sampler_pull(node_t * node, object_t ** output)
{
    object_t * inp_play = NULL;
    error_t e = node_pull(node, 0, &inp_play);
	if (e != SUCCESS) return e;

    struct state * state = &CAST_OBJECT(struct state, node->state);

    if (inp_play == NULL) {
        state->t = 0;
        *output = NULL;
        return SUCCESS;
    }

    long play = CAST_OBJECT(long, inp_play);
    double * chunk = &CAST_OBJECT(double, state->output);

    for (size_t i = 0; i < global_chunk_size; i++) {
        if (play && state->t < state->length)
            chunk[i] = state->sample[state->t++];
        else
            chunk[i] = 0;
    }

    if (!play)
        state->t = 0;

    *output = state->output;
    return SUCCESS;
}

node_t * sampler_create(const double * sample, size_t length)
{
    type_t * chunk_type = get_chunk_type();
    if (state_type == NULL) {
        state_type = make_object_and_pod_type(sizeof(struct state));
        if (state_type == NULL) return NULL;
    }

    node_t * node = node_alloc(1, 1, state_type);
    node->name = strdup("Sampler");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = long_type,
        .name = strdup("play?"),
    };

    // Define outputs
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .type = chunk_type,
        .name = strdup("out"),
        .pull = &sampler_pull,
    };

    // Init state
    struct state * state = &CAST_OBJECT(struct state, node->state);
    state->output = object_alloc(chunk_type);
    state->t = 0;
    state->length = length;
    state->sample = sample;
    
    return node;
}
