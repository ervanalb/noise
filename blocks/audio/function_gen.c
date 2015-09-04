#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

static double f(double t) {
	return sin(t*2*M_PI);
}

static enum pull_rc fungen_pull(struct port * port) {
    node_t * node = port->port_node;
    object_t * input0 = NODE_PULL(node, 0);

    if (input0 == NULL) {
        return PULL_RC_NULL;
    }

    CAST_OBJECT(double, port->port_value) = f(CAST_OBJECT(double, input0));
    return PULL_RC_OBJECT;
}

int fungen_init(node_t * node) {
    int rc = node_alloc_connections(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = strdup("Sine");

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = double_type,
        .inport_name = strdup("time"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("sin(t)"),
        .port_pull = &fungen_pull,
        .port_type = double_type,
        .port_value = object_alloc(double_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (node_term(node), -1);

    return 0;
}
