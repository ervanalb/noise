#include <stdlib.h>

#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "util.h"


static error_t wye_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e = node_pull(node, 0, &input0);

    *output = object_swap(&node->state, input0);

    for (size_t i = 1; i < node->n_inputs; i++) {
        object_t * inputx;
        e |= node_pull(node, i, &inputx);
    }

    return e;
}

node_t * wye_create(size_t n_inputs)
{
    if (n_inputs < 1) return NULL;

    node_t * node = node_alloc(n_inputs, 1, NULL);
    node->name = strdup("Wye");
    node->destroy = &node_destroy_generic;

    // Define inputs
    for (size_t i = 0; i < n_inputs; i++) {
        node->inputs[i] = (struct node_input) {
            .type = NULL,
            .name = (i == 0) ? strdup("main") : rsprintf("aux %lu", i),
        };
    }
    
    // Define outputs 
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &wye_pull,
        .type = NULL,
        .name = strdup("out"),
    };

    return node;
}

//

static error_t tee_pull_main(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e = node_pull(node, 0, &input0);

    *output = object_swap(&node->state, input0);

    return e;
}

static error_t tee_pull_aux(node_t * node, object_t ** output)
{
    *output = node->state;
    return SUCCESS;
}

node_t * tee_create(size_t n_outputs)
{
    if (n_outputs < 1) return NULL;

    node_t * node = node_alloc(1, n_outputs, NULL);
    node->name = strdup("Tee");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = NULL,
        .name = strdup("in"),
    };
    
    // Define outputs 
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &tee_pull_main,
        .type = NULL,
        .name = strdup("main"),
    };
    
    for (size_t i = 1; i < n_outputs; i++) {
        node->outputs[i] = (struct endpoint) {
            .node = node,
            .pull = &tee_pull_aux,
            .type = NULL,
            .name = rsprintf("aux %lu", i),
        };
    }

    return node;
}
