#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/audio/blocks.h"
#include "core/util.h"

struct state {
    double envelope;
    double env_decay;

};

static enum nz_pull_rc compressor_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    nz_obj_p inp_chunk = NZ_NODE_PULL(node, 0);

    struct state * state = (struct state *) node->node_state;

	if (inp_chunk == NULL) {
        state->envelope = 0.;
        return NZ_PULL_RC_NULL;
	}

    double * input = &(*(double*)inp_chunk);
    double * output = &(*(double*)port->port_value);

    double chunk_max = 0.;
    size_t max_idx = nz_chunk_size - 1;
    for (size_t i = 0; i < nz_chunk_size; i++) {
        if (fabs(input[i]) > chunk_max) {
            chunk_max = fabs(input[i]);
            max_idx = i;
        }
    }

    double start_envelope = state->envelope;
    double end_envelope;
    if (chunk_max >= state->envelope) {
        end_envelope = chunk_max;
    } else {
        end_envelope = fabs(state->envelope + state->env_decay * (chunk_max - state->envelope));
    }

    for (size_t i = 0; i < nz_chunk_size; i++) {
        double env = end_envelope;
        // This algorithm is a bit shakey, because there could be a local max
        // before the chunk max that will be > than env
        // but the final clipping stage helps deal with this
        if (i < max_idx) {
            // Interpolate between start and end envelopes
            env = start_envelope + (end_envelope - start_envelope) * ((double) i / (double) max_idx);
        }
        if (env < 1.0) {
            output[i] = input[i];
        } else {
            output[i] = input[i] / env;
        }
        // Clip
        if (output[i] > 1.0) output[i] = 1.0;
        if (output[i] < -1.0) output[i] = -1.0;
    }

    state->envelope = end_envelope;

    return NZ_PULL_RC_OBJECT;
}

int nz_compressor_init(struct nz_node * node) {
    //TODO: more inputs as paramters
    int rc = nz_node_alloc_ports(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = strdup("Compressor");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_chunk_type,
        .inport_name = strdup("chunk in"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &compressor_pull,
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
    
    // Params
    state->env_decay = 0.01;

    // State
    state->envelope = 0.;

    return 0;
}
