#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/audio/blocks.h"
#include "core/util.h"

struct state {
    struct nz_obj * sample;
	size_t t;
};

static void sampler_term(struct nz_node * node) {
    nz_node_free_ports(node);

    struct state * state = (struct state *) node->node_state;
    nz_obj_destroy(&state->sample);
    free(node->node_state);
}

static enum nz_pull_rc sampler_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    struct nz_obj * inp_cmd = NZ_NODE_PULL(node, 1);

    struct state * state = (struct state *) node->node_state;

    if (inp_cmd == NULL) {
        state->t = 0;
        return NZ_PULL_RC_NULL;
    }

    enum nz_sampler_command cmd = NZ_CAST(long, inp_cmd);

    if (cmd == NZ_SAMPLER_COMMAND_REFETCH || state->sample == NULL) {
        struct nz_obj * inp_sample = NZ_NODE_PULL(node, 0);

        nz_obj_destroy(&state->sample);
        state->sample = nz_obj_dup(inp_sample);
        state->t = 0;
        if(!state->sample)
            printf("failed to get sample %p\n", inp_sample);
        else
            printf("got sample len %lu\n", nz_vector_get_size(state->sample));
    }

    if (state->sample == NULL) 
        return NZ_PULL_RC_NULL;

    double * chunk = &NZ_CAST(double, port->port_value);

    if (cmd == NZ_SAMPLER_COMMAND_RESTART || cmd == NZ_SAMPLER_COMMAND_STOP)
        state->t = 0;

    size_t length = nz_vector_get_size(state->sample);
    if ((cmd == NZ_SAMPLER_COMMAND_PLAY || cmd == NZ_SAMPLER_COMMAND_RESTART) && state->t < length) {
        double * sample = NZ_CAST(double *, state->sample);

        for (size_t i = 0; i < nz_chunk_size; i++) {
            if (state->t < length)
                chunk[i] = sample[state->t++];
            else
                chunk[i] = 0;
        }
    } else { // SAMPLER_COMMAND_PAUSE, STOP & otherwise
        memset(chunk, 0, sizeof(double) * nz_chunk_size);
    }

    return NZ_PULL_RC_OBJECT;
}

int sampler_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &sampler_term;
    node->node_name = strdup("Sampler");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = sample_type,
        .inport_name = strdup("sample"),
    };
    node->node_inputs[1] = (struct nz_inport) {
        .inport_type = long_type,
        .inport_name = strdup("command"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &sampler_pull,
        .port_type = chunk_type,
        .port_value = nz_obj_create(chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (state == NULL) 
        return (nz_node_term(node), -1);
    
    state->t = 0;
    state->sample = NULL;
    
    return 0;
}
