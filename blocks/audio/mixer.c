#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/audio/blocks.h"
#include "core/util.h"

static enum nz_pull_rc mixer_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    double * out_chunk = &NZ_CAST(double, port->port_value);
    
    memset(out_chunk, 0, sizeof(double) * nz_chunk_size);
    
    for (size_t i = 0; i < node->node_n_inputs; ) {
        struct nz_obj * input_chunk = NZ_NODE_PULL(node, i++);
        struct nz_obj * input_gain = NZ_NODE_PULL(node, i++);

        if (input_chunk == NULL || input_gain == NULL) continue;

        double gain = NZ_CAST(double, input_gain);
        double * mix_chunk = &NZ_CAST(double, input_chunk);

        for (size_t j = 0; j < nz_chunk_size; j++) {
            out_chunk[j] += gain * mix_chunk[j];
        }
    }

    return NZ_PULL_RC_OBJECT;
}

int nz_mixer_init(struct nz_node * node, size_t n_channels) {
    int rc = nz_node_alloc_ports(node, 2 * n_channels, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = rsprintf("Mixer %lu", n_channels);

    // Define inputs
    for (size_t i = 0; i < n_channels; i++) {
        node->node_inputs[2 * i + 0] = (struct nz_inport) {
            .inport_type = nz_chunk_type,
            .inport_name = rsprintf("ch %lu", i),
        };
        node->node_inputs[2 * i + 1] = (struct nz_inport) {
            .inport_type = nz_double_type,
            .inport_name = rsprintf("gain %lu", i),
        };
    }
    
    // Define output 
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("mixout"),
        .port_pull = &mixer_pull,
        .port_type = nz_chunk_type,
        .port_value = nz_obj_create(nz_chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    return 0;
}

//

static enum nz_pull_rc cmixer_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node;
    double * output = &NZ_CAST(double, port->port_value);
    
    memset(output, 0, sizeof(double) * nz_chunk_size);
    
    for (size_t i = 0; i < node->node_n_inputs; ) {
        struct nz_obj * input_chunk = NZ_NODE_PULL(node, i++);
        struct nz_obj * input_gains = NZ_NODE_PULL(node, i++);

        if (input_chunk == NULL || input_gains == NULL) continue;

        double * chunk = &NZ_CAST(double, input_chunk);
        double * gains = &NZ_CAST(double, input_gains);

        for (size_t j = 0; j < nz_chunk_size; j++) {
            output[j] = gains[j] * chunk[j];
        }
    }

    return NZ_PULL_RC_OBJECT;
}

int nz_cmixer_init(struct nz_node * node, size_t n_channels) {
    int rc = nz_node_alloc_ports(node, 2 * n_channels, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic;
    node->node_name = rsprintf("CMixer %lu", n_channels);

    // Define inputs
    for (size_t i = 0; i < n_channels; i++) {
        node->node_inputs[2 * i + 0] = (struct nz_inport) {
            .inport_type = nz_chunk_type,
            .inport_name = rsprintf("ch %lu", i),
        };
        node->node_inputs[2 * i + 1] = (struct nz_inport) {
            .inport_type = nz_chunk_type,
            .inport_name = rsprintf("gain %lu", i),
        };
    }
    
    // Define output 
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("mixout"),
        .port_pull = &cmixer_pull,
        .port_type = nz_chunk_type,
        .port_value = nz_obj_create(nz_chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (nz_node_term(node), -1);

    return 0;
}
