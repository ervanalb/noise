#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

static enum pull_rc recorder_pull(struct port * port) {
    node_t * node = port->port_node;
    object_t * inp_len = NODE_PULL(node, 1);

    if (inp_len == NULL)
        return PULL_RC_NULL;

    size_t length = CAST_OBJECT(long, inp_len);
    size_t t = 0;

    free(port->port_value);
    port->port_value = object_alloc(get_sample_type());
    vector_set_size(port->port_value, length); //TODO this can fail
    double * samples = CAST_OBJECT(double *, port->port_value);

    while (t < length) {
        object_t * inp_chunk = NODE_PULL(node, 0);

        if (inp_chunk == NULL) {
            // We don't want to loop forever... so just increment t by one?
            // XXX
            assert(0);
            //t++;
            continue;
        }

        double * chunk = &CAST_OBJECT(double, inp_chunk);
        if (t + nz_chunk_size < length) {
            memcpy(&samples[t], chunk, sizeof(double) * nz_chunk_size);
            t += nz_chunk_size;
        } else {
            while(t < length)
                samples[t++] = *chunk++;
        }
    }
    return PULL_RC_OBJECT;
}

int recorder_init(node_t * node) {
    const struct type * chunk_type = get_chunk_type();
    const struct type * sample_type = get_sample_type();

    int rc = node_alloc_connections(node, 2, 1);
    if (rc != 0) return rc;

    node->node_name = strdup("Recorder");

    // Define inputs
    node->node_inputs[0] = (struct inport) {
        .inport_type = chunk_type,
        .inport_name = strdup("chunks"),
    };
    node->node_inputs[1] = (struct inport) {
        .inport_type = long_type,
        .inport_name = strdup("sizecmd"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("sample"),
        .port_pull = &recorder_pull,
        .port_type = sample_type,
        .port_value = NULL,
    };

    return 0;
}

