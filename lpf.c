#include <stdlib.h>
#include <math.h>
#include "globals.h"
#include "block.h"
#include "typefns.h"
#include "blockdef.h"
#include "util.h"

struct lpf_state {
    object_t * output;
	int active;
};

static type_t * lpf_state_type = NULL;

static error_t lpf_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    struct lpf_state * state = &CAST_OBJECT(struct lpf_state, node->state);

    object_t * inp_value = NULL;
    e |= node_pull(node, 0, &inp_value);

    object_t * inp_alpha = NULL;
    e |= node_pull(node, 1, &inp_alpha);

	if (inp_value == NULL) {
		state->active = 0;
		*output = NULL;
		return SUCCESS;
	}

    double cur_value = CAST_OBJECT(double, inp_value);
    double prev_value = CAST_OBJECT(double, state->output);
    double tau = (inp_alpha == NULL) ? 1.0 : CAST_OBJECT(double, inp_alpha);
    double alpha = exp(-tau);

	if (!state->active) {
		state->active = 1;
        CAST_OBJECT(double, state->output) = cur_value;
	} else {
        CAST_OBJECT(double, state->output) = prev_value + alpha * (cur_value - prev_value);
    }
    
	*output = state->output;

    return e;
}

node_t * lpf_create()
{
    if (lpf_state_type == NULL) {
        lpf_state_type = make_object_and_pod_type(sizeof(struct lpf_state));
        if (lpf_state_type == NULL) return NULL;
    }

    node_t * node = node_alloc(2, 1, lpf_state_type);
    node->name = strdup("LPF");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = double_type,
        .name = strdup("in"),
    };
    node->inputs[1] = (struct node_input) {
        .type = double_type,
        .name = strdup("alpha"),
    };
    
    // Define outputs
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &lpf_pull,
        .type = double_type,
        .name = strdup("out"),
    };

    // Initialize state
    
    CAST_OBJECT(struct lpf_state, node->state) = (struct lpf_state) {
        .output = object_alloc(double_type),
        .active = 0,
    };

    return node;
}
