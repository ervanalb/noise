#include <stdlib.h>

#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "util.h"


static int wye_pull(struct port * port) {
    node_t * node = port->port_node;
    object_t * out = object_swap(&port->port_value, NODE_PULL(node, 0));

    for (size_t i = 1; i < node->node_n_inputs; i++) {
        NODE_PULL(node, i);
    }

    return (out == NULL) ? PULL_RC_NULL : PULL_RC_OBJECT;
}

int wye_init(node_t * node, size_t n_inputs) {
    if (n_inputs < 1) return (errno = EINVAL, -1);

    int rc = node_alloc_connections(node, n_inputs, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = rsprintf("Wye %lu", n_inputs);

    // Define inputs
    for (size_t i = 0; i < n_inputs; i++) {
        node->node_inputs[i] = (struct inport) {
            .inport_type = NULL,
            .inport_name = (i == 0) ? strdup("main") : rsprintf("aux %lu", i),
        };
    }
    
    // Define outputs 
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &wye_pull,
        .port_type = NULL,
        .port_value = NULL,
    };

    return 0;
}

//

static enum pull_rc tee_pull_main(struct port * port) {
    node_t * node = port->port_node;
    object_t * out = object_swap(&port->port_value, NODE_PULL(node, 0));

    for (size_t i = 1; i < node->node_n_outputs; i++) {
        node->node_outputs[i].port_value = out;
    }
    
    return (out == NULL) ? PULL_RC_NULL : PULL_RC_OBJECT;
}

static enum pull_rc tee_pull_aux(struct port * port) {
    return PULL_RC_OBJECT;
}

int tee_init(node_t * node, size_t n_outputs) {
    if (n_outputs < 1) return (errno = EINVAL, -1);

    int rc = node_alloc_connections(node, 1, n_outputs);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = rsprintf("Tee %lu", n_outputs);

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = NULL,
        .inport_name = strdup("in"),
    };
    
    // Define outputs 
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("main"),
        .port_pull = &tee_pull_main,
        .port_type = NULL,
        .port_value = NULL,
    };
    
    for (size_t i = 1; i < n_outputs; i++) {
        node->node_outputs[i] = (struct port) {
            .port_node = node,
            .port_name = rsprintf("aux %lu", i),
            .port_pull = &tee_pull_aux,
            .port_type = NULL,
            .port_value = NULL,
        };
    }

    return 0;
}
