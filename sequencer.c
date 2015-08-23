#include <math.h>
#include <stdlib.h>
#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "globals.h"
#include "util.h"


static error_t sequencer_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * inp_time = NULL;
    e |= node_pull(node, 0, &inp_time);

    object_t * inp_stream = NULL;
    e |= node_pull(node, 1, &inp_stream);

    //TODO - null behavior
    if (inp_time == NULL || inp_stream == NULL) {
        *output = NULL;
        return e;
    }

    int t = (int) CAST_OBJECT(double, inp_time);

    size_t stream_idx = t % tuple_length(inp_stream);
    object_t * result = (&CAST_OBJECT(object_t *, (inp_stream)))[stream_idx];

    object_free(node->state);
    *output = node->state = object_dup(result);

    return e;
}

node_t * sequencer_create()
{
    node_t * node = node_alloc(2, 1, NULL);
    node->name = strdup("Sequencer");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = double_type,
        .name = strdup("time"),
    };
    node->inputs[1] = (struct node_input) {
        .type = NULL, // Takes in a tuple type...
        .name = strdup("tuple stream"),
    };
    
    // Define outputs
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &sequencer_pull,
        .type = NULL, // ...and returns elements from the tuple
        .name = strdup("out"),
    };

    return node;
}
