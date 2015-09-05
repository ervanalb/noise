#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"

static enum nz_pull_rc sequencer_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node; 

    struct nz_obj * inp_time = NZ_NODE_PULL(node, 0);
    struct nz_obj * inp_stream = NZ_NODE_PULL(node, 1);

    if (inp_time == NULL || inp_stream == NULL) {
        return NZ_PULL_RC_NULL;
    }

    int t = (int) NZ_CAST(double, inp_time);

    size_t stream_idx = t % nz_vector_get_size(inp_stream);
    struct nz_obj ** sequence = NZ_CAST(struct nz_obj **, inp_stream);
    struct nz_obj * out = nz_obj_swap(&port->port_value, sequence[stream_idx]);

    return (out == NULL) ? NZ_PULL_RC_NULL : NZ_PULL_RC_OBJECT;
}

int nz_sequencer_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = strdup("Sequencer");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_double_type,
        .inport_name = strdup("time"),
    };
    node->node_inputs[1] = (struct nz_inport) {
        .inport_type = nz_object_vector_type,
        .inport_name = strdup("sequence"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &sequencer_pull,
        .port_type = NULL,
        .port_value = NULL,
    };

    return 0;
}
