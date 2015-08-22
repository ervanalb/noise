#include <stdlib.h>
#include "block.h"
#include "blockdef.h"
#include "util.h"

static error_t constant_pull(node_t * node, object_t ** output)
{
    *output = node->state;
    return SUCCESS;
}

node_t * constant_create(object_t * constant_value)
{
    node_t * node = node_alloc(0, 1, object_type(constant_value));
    node->name = strdup("Constant");
    node->destroy = &node_destroy_generic;
    
    // Define outputs (0: double sum)
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = constant_pull,
        .type = object_type(constant_value),
        .name = strdup("constant"),
    };

    return node;
}

