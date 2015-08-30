#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

static enum pull_rc sequencer_pull(struct port * port) {
    node_t * node = port->port_node; 

    object_t * inp_time = NODE_PULL(node, 0);
    object_t * inp_stream = NODE_PULL(node, 1);

    if (inp_time == NULL || inp_stream == NULL) {
        return PULL_RC_NULL;
    }

    int t = (int) CAST_OBJECT(double, inp_time);

    size_t stream_idx = t % vector_get_size(inp_stream);
    object_t ** sequence = CAST_OBJECT(object_t **, inp_stream);
    object_t * out = object_swap(&port->port_value, sequence[stream_idx]);

    return (out == NULL) ? PULL_RC_NULL : PULL_RC_OBJECT;
}

int sequencer_init(node_t * node) {
    const struct type * object_vector_type = get_object_vector_type();

    int rc = node_alloc_connections(node, 2, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = strdup("Sequencer");

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = double_type,
        .inport_name = strdup("time"),
    };
    node->node_inputs[1] = (struct inport) {
        .inport_type = object_vector_type,
        .inport_name = strdup("sequence"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &sequencer_pull,
        .port_type = NULL,
        .port_value = NULL,
    };

    return 0;
}
