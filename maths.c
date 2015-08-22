#include <math.h>
#include <stdlib.h>
#include "block.h"
#include "blockdef.h"
#include "error.h"

static error_t add_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e |= pull(node, 0, &input0);

    object_t * input1 = NULL;
    e |= pull(node, 1, &input1);

    if (input0 == NULL || input1 == NULL) {
        output = NULL;
        return e;
    }

    double x = CAST_OBJECT(double, input0);
    double y = CAST_OBJECT(double, input1);

    // Do the magic here:
    double result = x + y;
    // --

    CAST_OBJECT(double, node->state) = result;
    *output = node->state;

    return e;
}

static error_t subtract_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e |= pull(node, 0, &input0);

    object_t * input1 = NULL;
    e |= pull(node, 1, &input1);

    if (input0 == NULL || input1 == NULL) {
        output = NULL;
        return e;
    }

    double x = CAST_OBJECT(double, input0);
    double y = CAST_OBJECT(double, input1);

    // Do the magic here:
    double result = x - y;
    // --

    CAST_OBJECT(double, node->state) = result;
    *output = node->state;

    return e;
}

static error_t multiply_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e |= pull(node, 0, &input0);

    object_t * input1 = NULL;
    e |= pull(node, 1, &input1);

    if (input0 == NULL || input1 == NULL) {
        output = NULL;
        return e;
    }

    double x = CAST_OBJECT(double, input0);
    double y = CAST_OBJECT(double, input1);

    // Do the magic here:
    double result = x * y;
    // --

    CAST_OBJECT(double, node->state) = result;
    *output = node->state;

    return e;
}

static error_t divide_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e |= pull(node, 0, &input0);

    object_t * input1 = NULL;
    e |= pull(node, 1, &input1);

    if (input0 == NULL || input1 == NULL) {
        output = NULL;
        return e;
    }

    double x = CAST_OBJECT(double, input0);
    double y = CAST_OBJECT(double, input1);

    // Do the magic here:
    double result = x / y;
    // --

    CAST_OBJECT(double, node->state) = result;
    *output = node->state;

    return e;
}


static error_t note_to_freq_pull(node_t * node, object_t ** output)
{
    error_t e = SUCCESS;
    object_t * input0 = NULL;
    e |= pull(node, 0, &input0);

    if (input0 == NULL) {
        output = NULL;
        return e;
    }

    double note = CAST_OBJECT(double, input0);
    CAST_OBJECT(double, node->state) = pow(2,(note-69)/12)*440;
    *output = node->state;

    return e;
}

 
node_t * math_create(enum math_op op)
{
    pull_fn_pt pull_fn;
    size_t n_inputs = 2;
    switch(op)
    {
    case MATH_ADD:
        pull_fn = &add_pull;
        break;
    case MATH_SUBTRACT:
        pull_fn = &subtract_pull;
        break;
    case MATH_MULTIPLY:
        pull_fn = &multiply_pull;
        break;
    case MATH_DIVIDE:
        pull_fn = &divide_pull;
        break;
    case MATH_NOTE_TO_FREQ:
        pull_fn = &note_to_freq_pull;
        n_inputs = 1;
        break;
    default:
        return NULL;
    }

    node_t * node = allocate_node(n_inputs, 1, double_type);
    node->destroy = &generic_block_destroy;
    
    // Define outputs (0: double sum)
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = pull_fn,
        .type = double_type,
        .name = "result",
    };

    return node;
}
