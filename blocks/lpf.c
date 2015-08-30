#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

struct state {
	int active;
};

static enum pull_rc lpf_pull(struct port * port) {
    node_t * node = port->port_node;

    object_t * inp_value = NODE_PULL(node, 0);
    object_t * inp_alpha = NODE_PULL(node, 1);

    struct state * state = (struct state *) node->node_state;

	if (inp_value == NULL) {
		state->active = 0;
        return PULL_RC_NULL;
	}

    double cur_value = CAST_OBJECT(double, inp_value);
    double prev_value = CAST_OBJECT(double, port->port_value);
    double tau = (inp_alpha == NULL) ? 1.0 : CAST_OBJECT(double, inp_alpha);
    double alpha = exp(-tau);

	if (!state->active) {
		state->active = 1;
        CAST_OBJECT(double, port->port_value) = cur_value;
	} else {
        CAST_OBJECT(double, port->port_value) = prev_value + alpha * (cur_value - prev_value);
    }

    return PULL_RC_OBJECT;
}

int lpf_init(node_t * node) {
    int rc = node_alloc_connections(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = strdup("LPF");

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = double_type,
        .inport_name = strdup("in"),
    };
    node->node_inputs[1] = (struct inport) {
        .inport_type = double_type,
        .inport_name = strdup("alpha"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &lpf_pull,
        .port_type = double_type,
        .port_value = object_alloc(double_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (state == NULL) 
        return (node_term(node), -1);
    
    state->active = 0;

    return 0;
}

//

static enum pull_rc clpf_pull(struct port * port) {
    node_t * node = port->port_node;

    object_t * inp_chunk = NODE_PULL(node, 0);
    object_t * inp_alpha = NODE_PULL(node, 1);

    struct state * state = (struct state *) node->node_state;

	if (inp_chunk == NULL) {
		state->active = 0;
        return PULL_RC_NULL;
	}

    double * input = &(CAST_OBJECT(double, inp_chunk));
    double * output = &(CAST_OBJECT(double, port->port_value));
    double tau = (inp_alpha == NULL) ? 1.0 : CAST_OBJECT(double, inp_alpha);
    double alpha = exp(-tau);

    double prev_value = 0.;
    if (state->active) {
        prev_value = output[noise_chunk_size-1];
    } else {
        state->active = 1;
        prev_value = input[0];
    }


    for (size_t i = 0; i < noise_chunk_size; i++) {
        prev_value = output[i] = prev_value + alpha * (input[i] - prev_value);
    }

    return PULL_RC_OBJECT;
}

int clpf_init(node_t * node) {
    const struct type * chunk_type = get_chunk_type();
    int rc = node_alloc_connections(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = strdup("LPF");

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = chunk_type,
        .inport_name = strdup("chunk in"),
    };
    node->node_inputs[1] = (struct inport) {
        .inport_type = double_type,
        .inport_name = strdup("alpha"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &clpf_pull,
        .port_type = chunk_type,
        .port_value = object_alloc(chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (state == NULL) 
        return (node_term(node), -1);
    
    state->active = 0;

    return 0;
}
