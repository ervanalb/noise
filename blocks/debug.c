#include <stdlib.h>
#include "block.h"
#include "blockdef.h"
#include "debug.h"
#include "noise.h"
#include "util.h"

struct state {
    char on;
	char name[32];
};

static enum pull_rc debug_pull(struct port * port){
    node_t * node = port->port_node;
    struct state * state = (struct state *) node->node_state;

    object_t * out = object_swap(&port->port_value, NODE_PULL(node, 0));

    if (state->on) {
        char * ostr = object_str(port->port_value);
        printf("%s: %s\n", state->name, ostr);
        free(ostr);
    }

    return (out == NULL) ? PULL_RC_NULL : PULL_RC_OBJECT;
}

int debug_init(node_t * node, const char * name, char on) {
    int rc = node_alloc_connections(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = rsprintf("Debug printer '%.32s'", name);

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = NULL,
        .inport_name = strdup("in"),
    };

    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = debug_pull,
        .port_type = NULL,
        .port_value = NULL,
    };

    // Init state
    node->node_state = calloc(1, sizeof(struct state));
    if (node->node_state == NULL) return (node_term(node), -1);

    struct state * state = (struct state *) node->node_state;
    state->on = on;
    strncpy(state->name, name, sizeof(state->name));

    return 0;
}

void debug_print_graph(node_t * node)
{
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
        struct port * port = node->node_inputs[i].inport_connection;
        if (port) 
            debug_print_graph(port->port_node);
    }

    for (size_t i = 0; i < node->node_n_outputs; i++) {
        //debug_print_graph(node->outputs[i].node);
    }
}
