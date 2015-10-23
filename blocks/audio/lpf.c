#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/audio/blocks.h"
#include "core/util.h"

struct state {
	int active;
};

static enum nz_pull_rc lpf_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    nz_obj_p inp_value = NZ_NODE_PULL(node, 0);
    nz_obj_p inp_alpha = NZ_NODE_PULL(node, 1);

    struct state * state = (struct state *) node->node_state;

	if (inp_value == NULL) {
		state->active = 0;
        return NZ_PULL_RC_NULL;
	}

    double cur_value = *(double*)inp_value;
    double prev_value = *(double*)port->port_value;
    double tau = (inp_alpha == NULL) ? 1.0 : *(double*)inp_alpha;
    double alpha = exp(-tau);

	if (!state->active) {
		state->active = 1;
        *(double*)port->port_value = cur_value;
	} else {
        *(double*)port->port_value = prev_value + alpha * (cur_value - prev_value);
    }

    return NZ_PULL_RC_OBJECT;
}

int nz_lpf_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = strdup("LPF");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_double_type,
        .inport_name = strdup("in"),
    };
    node->node_inputs[1] = (struct nz_inport) {
        .inport_type = nz_double_type,
        .inport_name = strdup("alpha"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &lpf_pull,
        .port_type = nz_double_type,
        .port_value = nz_obj_create(nz_double_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (state == NULL) 
        return (nz_node_term(node), -1);
    
    state->active = 0;

    return 0;
}

//

static enum nz_pull_rc clpf_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    nz_obj_p inp_chunk = NZ_NODE_PULL(node, 0);
    nz_obj_p inp_alpha = NZ_NODE_PULL(node, 1);

    struct state * state = (struct state *) node->node_state;

	if (inp_chunk == NULL) {
		state->active = 0;
        return NZ_PULL_RC_NULL;
	}

    double * input = &(*(double*)inp_chunk);
    double * output = &(*(double*)port->port_value);
    double tau = (inp_alpha == NULL) ? 1.0 : *(double*)inp_alpha;
    double alpha = exp(-tau);

    double prev_value = 0.;
    if (state->active) {
        prev_value = output[nz_chunk_size-1];
    } else {
        state->active = 1;
        prev_value = input[0];
    }


    for (size_t i = 0; i < nz_chunk_size; i++) {
        prev_value = output[i] = prev_value + alpha * (input[i] - prev_value);
    }

    return NZ_PULL_RC_OBJECT;
}

int nz_clpf_init(struct nz_node* node) {
    int rc = nz_node_alloc_ports(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = strdup("LPF");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_chunk_type,
        .inport_name = strdup("chunk in"),
    };
    node->node_inputs[1] = (struct nz_inport) {
        .inport_type = nz_double_type,
        .inport_name = strdup("alpha"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &clpf_pull,
        .port_type = nz_chunk_type,
        .port_value = nz_obj_create(nz_chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (state == NULL) 
        return (nz_node_term(node), -1);
    
    state->active = 0;

    return 0;
}
