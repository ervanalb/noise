#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/audio/blocks.h"
#include "core/util.h"

static enum nz_pull_rc recorder_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    struct nz_obj * inp_len = NZ_NODE_PULL(node, 1);

    if (inp_len == NULL)
        return NZ_PULL_RC_NULL;

    size_t length = NZ_CAST(long, inp_len);
    size_t t = 0;

    free(port->port_value);
    port->port_value = nz_obj_create(nz_sample_type);

    // Resize sample size & fail if there's a problem
    size_t new_length = nz_vector_set_size(port->port_value, length);
    if (new_length != length) return NZ_PULL_RC_ERROR;

    double * samples = NZ_CAST(double *, port->port_value);

    int count = 0;
    while (t < length) {
        struct nz_obj * inp_chunk = NZ_NODE_PULL(node, 0);

        if (inp_chunk == NULL) {
            // We don't want to loop forever, if `inp_chunk` continues to be NULL
            // TODO: how should this be handled? 
            fprintf(stderr, "Recorder pulled a null chunk; could loop forever; returning error.\n");
            return NZ_PULL_RC_ERROR;
            //assert(0);
            continue;
        }

        double * chunk = &NZ_CAST(double, inp_chunk);

        if (t + nz_chunk_size < length) {
            memcpy(&samples[t], chunk, sizeof(double) * nz_chunk_size);
            t += nz_chunk_size;
        } else {
            while(t < length)
                samples[t++] = *chunk++;
        }
    }
    return NZ_PULL_RC_OBJECT;
}

int nz_recorder_init(struct nz_node * node) {
    int rc = nz_node_alloc_ports(node, 2, 1);
    if (rc != 0) return rc;

    node->node_name = strdup("Recorder");

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_chunk_type,
        .inport_name = strdup("chunks"),
    };
    node->node_inputs[1] = (struct nz_inport) {
        .inport_type = nz_long_type,
        .inport_name = strdup("sizecmd"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("sample"),
        .port_pull = &recorder_pull,
        .port_type = nz_sample_type,
        .port_value = NULL,
    };

    return 0;
}

