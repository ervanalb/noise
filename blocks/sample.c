#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

struct state {
    object_t * sample;
	size_t t;
};

static void sampler_term(node_t * node) {
    node_free_connections(node);

    struct state * state = (struct state *) node->node_state;
    object_free(state->sample);
    free(node->node_state);
}

static enum pull_rc sampler_pull(struct port * port) {
    node_t * node = port->port_node;
    object_t * inp_cmd = NODE_PULL(node, 1);

    struct state * state = (struct state *) node->node_state;

    if (inp_cmd == NULL) {
        state->t = 0;
        return PULL_RC_NULL;
    }

    enum sampler_command cmd = CAST_OBJECT(long, inp_cmd);

    if (cmd == SAMPLER_COMMAND_REFETCH || state->sample == NULL) {
        object_t * inp_sample = NODE_PULL(node, 0);

        object_free(state->sample);
        state->sample = object_dup(inp_sample);
        state->t = 0;
        if(!state->sample)
            printf("failed to get sample %p\n", inp_sample);
        else
            printf("got sample len %lu\n", vector_get_size(state->sample));
    }

    if (state->sample == NULL) 
        return PULL_RC_NULL;

    double * chunk = &CAST_OBJECT(double, port->port_value);

    if (cmd == SAMPLER_COMMAND_RESTART || cmd == SAMPLER_COMMAND_STOP)
        state->t = 0;

    size_t length = vector_get_size(state->sample);
    if ((cmd == SAMPLER_COMMAND_PLAY || cmd == SAMPLER_COMMAND_RESTART) && state->t < length) {
        double * sample = CAST_OBJECT(double *, state->sample);

        for (size_t i = 0; i < noise_chunk_size; i++) {
            if (state->t < length)
                chunk[i] = sample[state->t++];
            else
                chunk[i] = 0;
        }
    } else { // SAMPLER_COMMAND_PAUSE, STOP & otherwise
        memset(chunk, 0, sizeof(double) * noise_chunk_size);
    }

    return PULL_RC_OBJECT;
}

int sampler_init(node_t * node) {
    const struct type * chunk_type = get_chunk_type();
    const struct type * sample_type = get_sample_type();

    int rc = node_alloc_connections(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &sampler_term;
    node->node_name = strdup("Sampler");

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = sample_type,
        .inport_name = strdup("sample"),
    };
    node->node_inputs[1] = (struct inport) {
        .inport_type = long_type,
        .inport_name = strdup("command"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &sampler_pull,
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
    
    state->t = 0;
    state->sample = NULL;
    
    return 0;
}
