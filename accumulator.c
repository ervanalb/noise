#include <stdlib.h>
#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "util.h"

static error_t accumulator_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e |= pull(node, 0, &input0);

    if (input0 != NULL) { //TODO: formalize null behavior?
        CAST_OBJECT(double, node->state) += CAST_OBJECT(double, input0);
    }

    *output = node->state;
    return e;
}

node_t * accumulator_create()
{
    node_t * node = node_alloc(1, 1, double_type);
    node->name = strdup("Accumulator");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = double_type,
        .name = strdup("delta"),
    };
    
    // Define outputs
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &accumulator_pull,
        .type = double_type,
        .name = strdup("sum"),
    };

    // Initialize state
    
    CAST_OBJECT(double, node->state) = 0.0;

    return node;
}

