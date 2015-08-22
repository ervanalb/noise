#include <stdlib.h>
#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "globals.h"

static error_t mixer_pull(node_t * node, object_t ** output)
{
    error_t e;
    
    for (size_t j = 0; j < global_chunk_size; j++) {
        CAST_OBJECT(double *, node->state)[j] = 0.;
    }
    
    for (size_t i = 0; i < node->n_inputs; ) {
        object_t * input_chunk = NULL;
        e |= pull(node, i++, &input_chunk);

        object_t * input_gain = NULL;
        e |= pull(node, i++, &input_gain);

        if (input_chunk == NULL || input_gain == NULL) continue;

        for (size_t j = 0; j < global_chunk_size; j++) {
            CAST_OBJECT(double *, node->state)[j] += CAST_OBJECT(double, input_gain) * CAST_OBJECT(double *, input_chunk)[j];
        }

    }

    *output = node->state;
    return e;
}

node_t * mixer_create(size_t n_channels)
{
    type_t * chunk_type = get_chunk_type();
    node_t * node = node_alloc(n_channels * 2, 1, chunk_type);
    node->name = "Mixer";
    node->destroy = &node_destroy_generic;
    
    // Define output 
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &mixer_pull,
        .type = double_type,
        .name = "mixout",
    };

    // Initialize state
    CAST_OBJECT(double, node->state) = 0.0;

    return node;
}
