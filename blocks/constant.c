#include <stdlib.h>
#include "block.h"
#include "blockdef.h"
#include "util.h"

static enum pull_rc constant_pull(struct port * port) {
    return PULL_RC_OBJECT;
}

int constant_init(node_t * node, object_t * constant_value) {
    int rc= node_alloc_connections(node, 0, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;

    char * constant_str = object_str(constant_value);
    node->node_name = rsprintf("Constant %s", constant_str);
    free(constant_str);
    
    // Define outputs (0: double sum)
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_pull = constant_pull,
        .port_type = object_type(constant_value),
        .port_name = strdup("constant"),
        .port_value = object_dup(constant_value),
    };

    return 0;
}

