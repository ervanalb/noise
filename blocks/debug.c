#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"
#include "debug.h" //TODO

struct state {
    char on;
	char name[32];
};

static enum nz_pull_rc debug_pull(struct nz_port * port){
    struct nz_node * node = port->port_node;
    struct state * state = (struct state *) node->node_state;

    struct nz_obj * out = nz_obj_swap(&port->port_value, NZ_NODE_PULL(node, 0));

    if (state->on) {
        char * ostr = nz_obj_str(port->port_value);
        printf("%s: %s\n", state->name, ostr);
        free(ostr);
    }

    return (out == NULL) ? NZ_PULL_RC_NULL : NZ_PULL_RC_OBJECT;
}

int nz_debug_init(struct nz_node * node, const char * name, char on) {
    int rc = nz_node_alloc_ports(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = rsprintf("Debug printer '%.32s'", name);

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = NULL,
        .inport_name = strdup("in"),
    };

    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = debug_pull,
        .port_type = NULL,
        .port_value = NULL,
    };

    // Init state
    node->node_state = calloc(1, sizeof(struct state));
    if (node->node_state == NULL) return (nz_node_term(node), -1);

    struct state * state = (struct state *) node->node_state;
    state->on = on;
    strncpy(state->name, name, sizeof(state->name));

    return 0;
}

void nz_debug_print_graph(struct nz_node * node) {
    printf("%p %s:\n", node, node->node_name);

    for (size_t i = 0; i < node->node_n_inputs; i++) {
        if (node->node_inputs[i].inport_connection != NULL) {
            printf("    [i] '%s' %s.'%s' (%p)\n",
                    node->node_inputs[i].inport_name,
                    node->node_inputs[i].inport_connection->port_node->node_name,
                    node->node_inputs[i].inport_connection->port_name,
                    node->node_inputs[i].inport_connection);
        } else {
            printf("    [i] '%s' --\n", node->node_inputs[i].inport_name);
        }
    }

    for (size_t i = 0; i < node->node_n_outputs; i++) {
        printf("    [o] %s\n", node->node_outputs[i].port_name);
    }
    
    for (size_t i = 0; i < node->node_n_inputs; i++) {
        struct nz_port * port = node->node_inputs[i].inport_connection;
        if (port) 
            nz_debug_print_graph(port->port_node);
    }

    for (size_t i = 0; i < node->node_n_outputs; i++) {
        //debug_print_graph(node->outputs[i].node);
    }
}
