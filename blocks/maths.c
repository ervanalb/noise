#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

static enum pull_rc add_pull(struct port * port) {
    node_t * node = port->port_node;

    object_t * input0 = NODE_PULL(node, 0);
    object_t * input1 = NODE_PULL(node, 1);

    if (input0 == NULL || input1 == NULL) {
        return PULL_RC_NULL;
    }

    double x = CAST_OBJECT(double, input0);
    double y = CAST_OBJECT(double, input1);

    // Do the magic here:
    double result = x + y;
    // --

    CAST_OBJECT(double, port->port_value) = result;
    return PULL_RC_OBJECT;
}

static enum pull_rc subtract_pull(struct port * port) {
    node_t * node = port->port_node;

    object_t * input0 = NODE_PULL(node, 0);
    object_t * input1 = NODE_PULL(node, 1);

    if (input0 == NULL || input1 == NULL) {
        return PULL_RC_NULL;
    }

    double x = CAST_OBJECT(double, input0);
    double y = CAST_OBJECT(double, input1);

    // Do the magic here:
    double result = x - y;
    // --

    CAST_OBJECT(double, port->port_value) = result;
    return PULL_RC_OBJECT;
}

static enum pull_rc multiply_pull(struct port * port) {
    node_t * node = port->port_node;

    object_t * input0 = NODE_PULL(node, 0);
    object_t * input1 = NODE_PULL(node, 1);

    if (input0 == NULL || input1 == NULL) {
        return PULL_RC_NULL;
    }

    double x = CAST_OBJECT(double, input0);
    double y = CAST_OBJECT(double, input1);

    // Do the magic here:
    double result = x * y;
    // --

    CAST_OBJECT(double, port->port_value) = result;
    return PULL_RC_OBJECT;
}

static enum pull_rc divide_pull(struct port * port) {
    node_t * node = port->port_node;

    object_t * input0 = NODE_PULL(node, 0);
    object_t * input1 = NODE_PULL(node, 1);

    if (input0 == NULL || input1 == NULL) {
        return PULL_RC_NULL;
    }

    double x = CAST_OBJECT(double, input0);
    double y = CAST_OBJECT(double, input1);

    // Do the magic here:
    double result = x / y;
    // --

    CAST_OBJECT(double, port->port_value) = result;
    return PULL_RC_OBJECT;
}

static enum pull_rc note_to_freq_pull(struct port * port) {
    node_t * node = port->port_node;

    object_t * input0 = NODE_PULL(node, 0);

    if (input0 == NULL) {
        return PULL_RC_NULL;
    }

    double note = CAST_OBJECT(double, input0);
    double freq = pow(2,(note-69)/12)*440;
    CAST_OBJECT(double, port->port_value) = freq;

    return PULL_RC_OBJECT;
}
 
int math_init(node_t * node, enum math_op op) {
    assert(node);
    port_pull_fn_pt pull_fn;
    size_t n_inputs = 2;
    const char * name;
    switch(op)
    {
    case MATH_ADD:
        pull_fn = &add_pull;
        name = "Add";
        break;
    case MATH_SUBTRACT:
        pull_fn = &subtract_pull;
        name = "Subtract";
        break;
    case MATH_MULTIPLY:
        pull_fn = &multiply_pull;
        name = "Multiply";
        break;
    case MATH_DIVIDE:
        pull_fn = &divide_pull;
        name = "Divide";
        break;
    case MATH_NOTE_TO_FREQ:
        pull_fn = &note_to_freq_pull;
        name = "Note to Freq.";
        n_inputs = 1;
        break;
    default:
        return (errno = EINVAL, -1);
    }
    
    int rc = node_alloc_connections(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = strdup(name);
    
    // Define inputs
    if (n_inputs == 1) {
        node->node_inputs[0] = (struct inport) {
            .inport_type = double_type,
            .inport_name = strdup("freq"),
        };
    } else {
        node->node_inputs[0] = (struct inport) {
            .inport_type = double_type,
            .inport_name = strdup("x"),
        };
        node->node_inputs[0] = (struct inport) {
            .inport_type = double_type,
            .inport_name = strdup("y"),
        };
    }

    // Define outputs (0: double sum)
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("result"),
        .port_pull = pull_fn,
        .port_type = double_type,
        .port_value = object_alloc(double_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (node_term(node), -1);

    return 0;
}
