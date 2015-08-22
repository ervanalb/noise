#include <stdlib.h>
#include "error.h"
#include "block.h"
#include "blockdef.h"

static error_t accumulator_pull(node_t * node, object_t ** output)
{
    error_t e;
    object_t * input0;
    if ((e = pull(node, 0, &input0) )) return e;

    if (input0 != NULL) { //TODO: formalize null behavior?
        CAST_OBJECT(double, node->state) += CAST_OBJECT(double, input0);
    }

    *output = node->state;
    return SUCCESS;
}

node_t * accumulator_create()
{
    node_t * node = allocate_node(1, 1, double_type);
    node->destroy = &generic_block_destroy;
    
    // Define outputs (0: double sum)
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &accumulator_pull,
        .type = double_type,
        .name = "sum",
    };

    // Initialize state
    
    CAST_OBJECT(double, node->state) = 0.0;

    return node;
}

