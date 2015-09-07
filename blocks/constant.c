#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"

static enum nz_pull_rc constant_pull(struct nz_port * port) {
    return NZ_PULL_RC_OBJECT;
}

int nz_constant_init(struct nz_node * node, struct nz_obj * constant_value) {
    int rc = nz_node_alloc_ports(node, 0, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;

    char * constant_str = nz_obj_str(constant_value);
    node->node_name = rsprintf("Constant %s", constant_str);
    free(constant_str);
    
    // Define outputs (0: double sum)
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_pull = constant_pull,
        .port_type = nz_obj_type(constant_value),
        .port_name = strdup("constant"),
        //.port_value = nz_obj_dup(constant_value),
        .port_value = constant_value,
    };

    return 0;
}

