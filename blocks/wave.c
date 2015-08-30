#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

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

static enum pull_rc wave_pull(struct port * port) {
    node_t * node = port->port_node;

    object_t * inp_freq = NODE_PULL(node, 0);
    object_t * inp_wave = NODE_PULL(node, 1);

    struct state * state = (struct state *) node->node_state;
    double * chunk = &CAST_OBJECT(double, port->port_value);

	if (inp_freq == NULL || inp_wave == NULL) {
        state->phase = 0.;
        memset(chunk, 0, noise_chunk_size * sizeof(double));
        return PULL_RC_OBJECT;
	}

    double freq = CAST_OBJECT(double, inp_freq);
    enum wave_type wave = CAST_OBJECT(long, inp_wave);

	for (size_t i=0; i < noise_chunk_size; i++) {
        switch(wave) {
            case WAVE_SINE:
                chunk[i] = sine(state->phase);
                break;
            case WAVE_SAW:
                chunk[i] = saw(state->phase);
                break;
            case WAVE_SQUARE:
                chunk[i] = square(state->phase);
                break;
            default:
                chunk[i] = 0.;
                break;
        }

        state->phase = fmod(state->phase + freq / noise_frame_rate, 1.0);
	}

    return PULL_RC_OBJECT;
}

int wave_init(node_t * node) {
    const struct type * chunk_type = get_chunk_type();
    int rc = node_alloc_connections(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = strdup("Wave");

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = double_type,
        .inport_name = strdup("freq"),
    };
    node->node_inputs[1] = (struct inport) {
        .inport_type = long_type,
        .inport_name = strdup("type"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &wave_pull,
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
    
    state->phase = 0.;

    return 0;
}

static enum pull_rc white_pull(struct port * port) {
    double * chunk = &CAST_OBJECT(double, port->port_value);

	for (size_t i=0; i < noise_chunk_size; i++) {
        chunk[i] = (rand() / (double) (RAND_MAX / 2)) - 1.0;
	}

    return PULL_RC_OBJECT;
}

int white_init(node_t * node) {
    const struct type * chunk_type = get_chunk_type();
    int rc = node_alloc_connections(node, 0, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = strdup("White noise");

    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &white_pull,
        .port_type = chunk_type,
        .port_value = object_alloc(chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (node_term(node), -1);

    return 0;
}
