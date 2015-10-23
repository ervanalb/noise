#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/audio/blocks.h"
#include "core/util.h"

static double f(double t) {
	return sin(t*2*M_PI);
}

static enum nz_pull_rc fungen_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    nz_obj_p input0 = NZ_NODE_PULL(node, 0);

    if (input0 == NULL) {
        return NZ_PULL_RC_NULL;
    }

    *(double*)port->port_value = f(*(double*)input0);
    return NZ_PULL_RC_OBJECT;
}

int nz_fungen_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = strdup("Sine");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_double_type,
        .inport_name = strdup("time"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("sin(t)"),
        .port_pull = &fungen_pull,
        .port_type = nz_double_type,
        .port_value = nz_obj_create(nz_double_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    return 0;
}
