#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

enum status {
    STATUS_NEW,
    STATUS_WAIT_HIGH,
    STATUS_WAIT_LOW
};

struct state {
    enum status status;
};

static enum pull_rc impulse_pull(struct port * port) {
    node_t * node = port->port_node;
    object_t * input0 = NODE_PULL(node, 0);

    struct state * state = (struct state *) node->node_state;
    CAST_OBJECT(double, port->port_value) = 0.0;

    switch(state->status) {
    case STATUS_NEW:
        CAST_OBJECT(double, port->port_value) = 1.0;
        state->status = STATUS_WAIT_LOW;
        break;
    case STATUS_WAIT_HIGH:
        if (input0 != NULL && CAST_OBJECT(double, input0)) {
            CAST_OBJECT(double, port->port_value) = 1.0;
            state->status = STATUS_WAIT_LOW;
        }
        break;
    case STATUS_WAIT_LOW:
        if (input0 == NULL || !CAST_OBJECT(double, input0)) {
            state->status = STATUS_WAIT_HIGH;
        }
        break;
    }

    return PULL_RC_OBJECT;
}

int impulse_init(node_t * node) {
    int rc = node_alloc_connections(node, 1, 1);
    if (rc != 0) return rc;

    node->node_name = strdup("Impulse");
    node->node_term = &node_term_generic;

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = double_type,
        .inport_name = strdup("trigger"),
    };
    
    // Define outputs
    const struct type * chunk_type = get_chunk_type();
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("impulse"),
        .port_pull = &impulse_pull,
        .port_type = chunk_type,
        .port_value = object_alloc(chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (node->node_state == NULL) 
        return (node_term(node), -1);
    
    state->status = STATUS_NEW;

    return 0;
}

