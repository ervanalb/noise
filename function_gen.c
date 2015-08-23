#include <math.h>
#include <stdlib.h>

#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "util.h"

static double f(double t)
{
	return sin(t*2*M_PI);
}

static error_t fungen_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e |= node_pull(node, 0, &input0);

    if (input0 == NULL) { //TODO: formalize null behavior?
        *output = NULL;
        return e;
    }

    CAST_OBJECT(double, node->state) = f(CAST_OBJECT(double, input0));
    *output = node->state;

    return e;
}

node_t * fungen_create()
{
    node_t * node = node_alloc(1, 1, double_type);
    node->name = strdup("Sine");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = double_type,
        .name = strdup("time"),
    };
    
    // Define outputs
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &fungen_pull,
        .type = double_type,
        .name = strdup("sin(t)"),
    };

    return node;
}
