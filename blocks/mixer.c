#include <math.h>
#include <stdlib.h>

#include "block.h"
#include "blockdef.h"
#include "noise.h"
#include "util.h"

static enum pull_rc mixer_pull(struct port * port) {
    node_t * node = port->port_node;
    
    for (size_t j = 0; j < noise_chunk_size; j++) {
        (&CAST_OBJECT(double, port->port_value))[j] = 0.;
    }
    
    for (size_t i = 0; i < node->node_n_inputs; ) {
        object_t * input_chunk = NODE_PULL(node, i++);
        object_t * input_gain = NODE_PULL(node, i++);

        if (input_chunk == NULL || input_gain == NULL) continue;

        for (size_t j = 0; j < noise_chunk_size; j++) {
            (&CAST_OBJECT(double, port->port_value))[j] += CAST_OBJECT(double, input_gain) * (&CAST_OBJECT(double, input_chunk))[j];
        }
    }

    return PULL_RC_OBJECT;
}

int mixer_init(node_t * node, size_t n_channels) {
    int rc = node_alloc_connections(node, 2 * n_channels, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = rsprintf("Mixer %lu", n_channels);

    const struct type * chunk_type = get_chunk_type();

    // Define inputs
    for (size_t i = 0; i < n_channels; i++) {
        node->node_inputs[2 * i + 0] = (struct inport) {
            .inport_type = chunk_type,
            .inport_name = rsprintf("ch %lu", i),
        };
        node->node_inputs[2 * i + 1] = (struct inport) {
            .inport_type = double_type,
            .inport_name = rsprintf("gain %lu", i),
        };
    }
    
    // Define output 
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("mixout"),
        .port_pull = &mixer_pull,
        .port_type = chunk_type,
        .port_value = object_alloc(chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (node_term(node), -1);

    return 0;
}

//

static enum pull_rc cmixer_pull(struct port * port) {
    node_t * node = port->port_node;
    double * output = &CAST_OBJECT(double, port->port_value);
    
    memset(output, 0, sizeof(double) * noise_chunk_size);
    
    for (size_t i = 0; i < node->node_n_inputs; ) {
        object_t * input_chunk = NODE_PULL(node, i++);
        object_t * input_gains = NODE_PULL(node, i++);

        if (input_chunk == NULL || input_gains == NULL) continue;

        double * chunk = &CAST_OBJECT(double, input_chunk);
        double * gains = &CAST_OBJECT(double, input_gains);

        for (size_t j = 0; j < noise_chunk_size; j++) {
            output[j] = gains[j] * chunk[j];
        }
    }

    return PULL_RC_OBJECT;
}

int cmixer_init(node_t * node, size_t n_channels) {
    int rc = node_alloc_connections(node, 2 * n_channels, 1);
    if (rc != 0) return rc;

    node->node_term = &node_term_generic;
    node->node_name = rsprintf("CMixer %lu", n_channels);

    const struct type * chunk_type = get_chunk_type();

    // Define inputs
    for (size_t i = 0; i < n_channels; i++) {
        node->node_inputs[2 * i + 0] = (struct inport) {
            .inport_type = chunk_type,
            .inport_name = rsprintf("ch %lu", i),
        };
        node->node_inputs[2 * i + 1] = (struct inport) {
            .inport_type = chunk_type,
            .inport_name = rsprintf("gain %lu", i),
        };
    }
    
    // Define output 
    node->node_outputs[0] = (struct port) {
        .port_node = node,
        .port_name = strdup("mixout"),
        .port_pull = &cmixer_pull,
        .port_type = chunk_type,
        .port_value = object_alloc(chunk_type),
    };

    if (node->node_outputs[0].port_value == NULL)
        return (node_term(node), -1);

    return 0;
}
