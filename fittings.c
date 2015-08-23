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

    if (input0 != NULL) {
        e |= object_copy(node->state, input0);
        *output = node->state;
    } else {
        *output = NULL;
    }

    for (size_t i = 1; i < node->n_inputs; i++) {
        object_t * inputx;
        e |= node_pull(node, i, &inputx);
    }

    return e;
}

node_t * wye_create(const type_t * type, size_t n_inputs)
{
    if (n_inputs < 1) return NULL;

    node_t * node = node_alloc(n_inputs, 1, type);
    node->name = strdup("Wye");
    node->destroy = &node_destroy_generic;

    // Define inputs
    for (size_t i = 0; i < n_inputs; i++) {
        node->inputs[i] = (struct node_input) {
            .type = type,
            .name = (i == 0) ? strdup("main") : strdup("aux"),
        };
    }
    
    // Define outputs 
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &wye_pull,
        .type = type,
        .name = strdup("first"),
    };

    return node;
}

//

static error_t tee_pull_main(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e = node_pull(node, 0, &input0);

    if (input0 != NULL) {
        e |= object_copy(node->state, input0);
        *output = node->state;
    } else {
        *output = NULL;
    }

    return e;
}

static error_t tee_pull_aux(node_t * node, object_t ** output)
{
    *output = node->state;
    return SUCCESS;
}

node_t * tee_create(const type_t * type, size_t n_outputs)
{
    if (n_outputs < 1) return NULL;

    node_t * node = node_alloc(1, n_outputs, type);
    node->name = strdup("Tee");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = type,
        .name = strdup("tee in"),
    };
    
    // Define outputs 
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &tee_pull_main,
        .type = type,
        .name = strdup("tee main"),
    };
    
    for (size_t i = 1; i < n_outputs; i++) {
        node->outputs[i] = (struct endpoint) {
            .node = node,
            .pull = &tee_pull_aux,
            .type = type,
            .name = strdup("tee aux"),
        };
    }

    return node;
}
