#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"

static enum nz_pull_rc add_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    struct nz_obj * input0 = NZ_NODE_PULL(node, 0);
    struct nz_obj * input1 = NZ_NODE_PULL(node, 1);

    if (input0 == NULL || input1 == NULL) {
        return NZ_PULL_RC_NULL;
    }

    double x = NZ_CAST(double, input0);
    double y = NZ_CAST(double, input1);

    // Do the magic here:
    double result = x + y;
    // --

    NZ_CAST(double, port->port_value) = result;
    return NZ_PULL_RC_OBJECT;
}

static enum nz_pull_rc subtract_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    struct nz_obj * input0 = NZ_NODE_PULL(node, 0);
    struct nz_obj * input1 = NZ_NODE_PULL(node, 1);

    if (input0 == NULL || input1 == NULL) {
        return NZ_PULL_RC_NULL;
    }

    double x = NZ_CAST(double, input0);
    double y = NZ_CAST(double, input1);

    // Do the magic here:
    double result = x - y;
    // --

    NZ_CAST(double, port->port_value) = result;
    return NZ_PULL_RC_OBJECT;
}

static enum nz_pull_rc multiply_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    struct nz_obj * input0 = NZ_NODE_PULL(node, 0);
    struct nz_obj * input1 = NZ_NODE_PULL(node, 1);

    if (input0 == NULL || input1 == NULL) {
        return NZ_PULL_RC_NULL;
    }

    double x = NZ_CAST(double, input0);
    double y = NZ_CAST(double, input1);

    // Do the magic here:
    double result = x * y;
    // --

    NZ_CAST(double, port->port_value) = result;
    return NZ_PULL_RC_OBJECT;
}

static enum nz_pull_rc divide_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    struct nz_obj * input0 = NZ_NODE_PULL(node, 0);
    struct nz_obj * input1 = NZ_NODE_PULL(node, 1);

    if (input0 == NULL || input1 == NULL) {
        return NZ_PULL_RC_NULL;
    }

    double x = NZ_CAST(double, input0);
    double y = NZ_CAST(double, input1);

    // Do the magic here:
    double result = x / y;
    // --

    NZ_CAST(double, port->port_value) = result;
    return NZ_PULL_RC_OBJECT;
}

static enum nz_pull_rc note_to_freq_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;

    struct nz_obj * input0 = NZ_NODE_PULL(node, 0);

    if (input0 == NULL) {
        return NZ_PULL_RC_NULL;
    }

    double note = NZ_CAST(double, input0);
    double freq = nz_note_to_freq(note);
    NZ_CAST(double, port->port_value) = freq;

    return NZ_PULL_RC_OBJECT;
}
 
int nz_math_init(struct nz_node * node, enum nz_math_op op) {
    if (node == NULL) return (errno = EINVAL, -1);

    size_t n_inputs = 2;
    switch(op) {
        case NZ_MATH_ADD:
        case NZ_MATH_SUBTRACT:
        case NZ_MATH_MULTIPLY:
        case NZ_MATH_DIVIDE:
            n_inputs = 2;
            break;
        case NZ_MATH_NOTE_TO_FREQ:
            n_inputs = 1;
            break;
        default:
            return (errno = EINVAL, -1);
    }
    
    int rc = nz_node_alloc_ports(node, n_inputs, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    
    // Define inputs
    if (n_inputs == 1) {
        node->node_inputs[0] = (struct nz_inport) {
            .inport_type = nz_double_type,
            .inport_name = strdup("freq"),
        };
    } else {
        node->node_inputs[0] = (struct nz_inport) {
            .inport_type = nz_double_type,
            .inport_name = strdup("x"),
        };
        node->node_inputs[1] = (struct nz_inport) {
            .inport_type = nz_double_type,
            .inport_name = strdup("y"),
        };
    }

    // Define outputs (0: double sum)
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("result"),
        .port_type = nz_double_type,
        .port_value = nz_obj_create(nz_double_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    switch(op) {
        case NZ_MATH_ADD:
            node->node_name = strdup("Add");
            node->node_outputs[0].port_pull = &add_pull;
            break;
        case NZ_MATH_SUBTRACT:
            node->node_name = strdup("Subtract");
            node->node_outputs[0].port_pull = &subtract_pull;
            break;
        case NZ_MATH_MULTIPLY:
            node->node_name = strdup("Multiply");
            node->node_outputs[0].port_pull = &multiply_pull;
            break;
        case NZ_MATH_DIVIDE:
            node->node_name = strdup("Divide");
            node->node_outputs[0].port_pull = &divide_pull;
            break;
        case NZ_MATH_NOTE_TO_FREQ:
            node->node_name = strdup("Note to Freq.");
            node->node_outputs[0].port_pull = &note_to_freq_pull;
            break;
        default:
            // This was already checked, won't actually happen
            assert(0);
    }

    return 0;
}
