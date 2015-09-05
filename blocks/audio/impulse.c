#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/audio/blocks.h"
#include "core/util.h"

enum status {
    STATUS_NEW,
    STATUS_WAIT_HIGH,
    STATUS_WAIT_LOW
};

struct state {
    enum status status;
};

static enum nz_pull_rc impulse_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    struct nz_obj * input0 = NZ_NODE_PULL(node, 0);

    struct state * state = (struct state *) node->node_state;
    NZ_CAST(double, port->port_value) = 0.0;

    switch(state->status) {
    case STATUS_NEW:
        NZ_CAST(double, port->port_value) = 1.0;
        state->status = STATUS_WAIT_LOW;
        break;
    case STATUS_WAIT_HIGH:
        if (input0 != NULL && NZ_CAST(double, input0)) {
            NZ_CAST(double, port->port_value) = 1.0;
            state->status = STATUS_WAIT_LOW;
        }
        break;
    case STATUS_WAIT_LOW:
        if (input0 == NULL || !NZ_CAST(double, input0)) {
            state->status = STATUS_WAIT_HIGH;
        }
        break;
    }

    return NZ_PULL_RC_OBJECT;
}

int nz_impulse_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 1, 1);
    if (rc != 0) return rc;

    node->node_name = strdup("Impulse");
    node->node_term = &nz_node_term_generic;

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = double_type,
        .inport_name = strdup("trigger"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("impulse"),
        .port_pull = &impulse_pull,
        .port_type = chunk_type,
        .port_value = nz_obj_create(chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (node->node_state == NULL) 
        return (nz_node_term(node), -1);
    
    state->status = STATUS_NEW;

    return 0;
}

