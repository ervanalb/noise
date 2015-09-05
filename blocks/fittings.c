#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"


static enum nz_pull_rc wye_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    struct nz_obj * out = nz_obj_swap(&port->port_value, NZ_NODE_PULL(node, 0));

    for (size_t i = 1; i < node->node_n_inputs; i++) {
        NZ_NODE_PULL(node, i);
    }

    return (out == NULL) ? NZ_PULL_RC_NULL : NZ_PULL_RC_OBJECT;
}

int nz_wye_init(struct nz_node * node, size_t n_inputs) {
    if (n_inputs < 1) return (errno = EINVAL, -1);

    int rc = nz_node_alloc_ports(node, n_inputs, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = rsprintf("Wye %lu", n_inputs);

    // Define inputs
    for (size_t i = 0; i < n_inputs; i++) {
        node->node_inputs[i] = (struct nz_inport) {
            .inport_type = NULL,
            .inport_name = (i == 0) ? strdup("main") : rsprintf("aux %lu", i),
        };
    }
    
    // Define outputs 
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &wye_pull,
        .port_type = NULL,
        .port_value = NULL,
    };

    return 0;
}

//

static enum nz_pull_rc tee_pull_main(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    struct nz_obj * out = nz_obj_swap(&port->port_value, NZ_NODE_PULL(node, 0));

    for (size_t i = 1; i < node->node_n_outputs; i++) {
        node->node_outputs[i].port_value = out;
    }
    
    return (out == NULL) ? NZ_PULL_RC_NULL : NZ_PULL_RC_OBJECT;
}

static enum nz_pull_rc tee_pull_aux(struct nz_port * port) {
    return NZ_PULL_RC_OBJECT;
}

int nz_tee_init(struct nz_node * node, size_t n_outputs) {
    if (n_outputs < 1) return (errno = EINVAL, -1);

    int rc = nz_node_alloc_ports(node, 1, n_outputs);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = rsprintf("Tee %lu", n_outputs);

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = NULL,
        .inport_name = strdup("in"),
    };
    
    // Define outputs 
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("main"),
        .port_pull = &tee_pull_main,
        .port_type = NULL,
        .port_value = NULL,
    };
    
    for (size_t i = 1; i < n_outputs; i++) {
        node->node_outputs[i] = (struct nz_port) {
            .port_node = node,
            .port_name = rsprintf("aux %lu", i),
            .port_pull = &tee_pull_aux,
            .port_type = NULL,
            .port_value = NULL,
        };
    }

    return 0;
}
