#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"

static enum nz_pull_rc accumulator_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    struct nz_obj * input0 = NZ_NODE_PULL(node, 0);

    if (input0 != NULL) {
        NZ_CAST(double, port->port_value) += NZ_CAST(double, input0);
    } else {
        NZ_CAST(double, port->port_value) = 0.0;
    }

    return NZ_PULL_RC_OBJECT;
}

int nz_accumulator_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = strdup("Accumulator");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = double_type,
        .inport_name = strdup("delta"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("sum"),
        .port_pull = &accumulator_pull,
        .port_type = double_type,
        .port_value = nz_obj_create(double_type)
    };

    // Initialize state
    NZ_CAST(double, NZ_NODE_OUTPUT(node, 0)) = 0.0;

    return 0;
}

