#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/audio/blocks.h"
#include "core/util.h"

struct state {
	double phase;
};

static double sine(double phi) {
	return sin(phi * 2 * M_PI);
}

static double saw(double phi) {
	return 2 * phi - 1;
}

static double square(double phi) {
	return 2 * (phi < .5) - 1;
}

static enum nz_pull_rc wave_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    struct nz_obj * inp_freq = NZ_NODE_PULL(node, 0);
    struct nz_obj * inp_wave = NZ_NODE_PULL(node, 1);

    struct state * state = (struct state *) node->node_state;
    double * chunk = &NZ_CAST(double, port->port_value);

	if (inp_freq == NULL || inp_wave == NULL) {
        state->phase = 0.;
        memset(chunk, 0, nz_chunk_size * sizeof(double));
        return NZ_PULL_RC_OBJECT;
	}

    double freq = NZ_CAST(double, inp_freq);
    enum nz_wave_type wave = NZ_CAST(long, inp_wave);

	for (size_t i=0; i < nz_chunk_size; i++) {
        switch(wave) {
            case NZ_WAVE_SINE:
                chunk[i] = sine(state->phase);
                break;
            case NZ_WAVE_SAW:
                chunk[i] = saw(state->phase);
                break;
            case NZ_WAVE_SQUARE:
                chunk[i] = square(state->phase);
                break;
            default:
                chunk[i] = 0.;
                break;
        }

        state->phase = fmod(state->phase + freq / nz_frame_rate, 1.0);
	}

    return NZ_PULL_RC_OBJECT;
}

int nz_wave_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = strdup("Wave");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_double_type,
        .inport_name = strdup("freq"),
    };
    node->node_inputs[1] = (struct nz_inport) {
        .inport_type = nz_long_type,
        .inport_name = strdup("type"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &wave_pull,
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
    
    state->phase = 0.;

    return 0;
}

static enum nz_pull_rc white_pull(struct nz_port * port) {
    double * chunk = &NZ_CAST(double, port->port_value);

	for (size_t i=0; i < nz_chunk_size; i++) {
        chunk[i] = (rand() / (double) (RAND_MAX / 2)) - 1.0;
	}

    return NZ_PULL_RC_OBJECT;
}

int nz_white_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 0, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = strdup("White noise");

    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &white_pull,
        .port_type = nz_chunk_type,
        .port_value = nz_obj_create(nz_chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    return 0;
}
