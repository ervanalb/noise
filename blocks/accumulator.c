#include <stdlib.h>
#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

static enum pull_rc accumulator_pull(struct port * port) {
    node_t * node = port->port_node;
    object_t * input0 = NODE_PULL(node, 0);

    if (input0 != NULL) {
        CAST_OBJECT(double, port->port_value) += CAST_OBJECT(double, input0);
    } else {
        CAST_OBJECT(double, port->port_value) = 0.0;
    }

    return PULL_RC_OBJECT;
}

int accumulator_init(node_t * node) {
    int rc = node_alloc_connections(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = strdup("Accumulator");

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = double_type,
        .inport_name = strdup("delta"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("sum"),
        .port_pull = &accumulator_pull,
        .port_type = double_type,
        .port_value = object_alloc(double_type)
    };

    // Initialize state
    CAST_OBJECT(double, NODE_OUTPUT(node, 0)) = 0.0;

    return 0;
}

