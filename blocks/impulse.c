#include <stdlib.h>
#include "error.h"
#include "block.h"
#include "blockdef.h"
#include "util.h"

enum status {
    STATUS_NEW,
    STATUS_WAIT_HIGH,
    STATUS_WAIT_LOW
};

struct state {
    object_t * output;
    enum status status;
};

static type_t * state_type;

static error_t impulse_pull(node_t * node, object_t ** output)
{
    object_t * input0 = NULL;
    node_pull(node, 0, &input0);

    struct state * state = &CAST_OBJECT(struct state, node->state);
    CAST_OBJECT(double, state->output) = 0.0;

    switch(state->status) {
    case STATUS_NEW:
        CAST_OBJECT(double, state->output) = 1.0;
        state->status = STATUS_WAIT_LOW;
        break;
    case STATUS_WAIT_HIGH:
        if (input0 != NULL && CAST_OBJECT(double, input0)) {
            CAST_OBJECT(double, state->output) = 1.0;
            state->status = STATUS_WAIT_LOW;
        }
        break;
    case STATUS_WAIT_LOW:
        if (input0 == NULL || !CAST_OBJECT(double, input0))
            state->status = STATUS_WAIT_HIGH;
        break;
    }

    *output = state->output;
    return SUCCESS;
}

node_t * impulse_create()
{
    if (state_type == NULL) {
        state_type = make_object_and_pod_type(sizeof(struct state));
        if (state_type == NULL) return NULL;
    }

    node_t * node = node_alloc(1, 1, state_type);
    node->name = strdup("Impulse");
    node->destroy = &node_destroy_generic;

    // Define inputs
    node->inputs[0] = (struct node_input) {
        .type = double_type,
        .name = strdup("trigger"),
    };
    
    // Define outputs
    node->outputs[0] = (struct endpoint) {
        .node = node,
        .pull = &impulse_pull,
        .type = chunk_type,
        .name = strdup("impulse"),
    };

    // Initialize state
    
    struct state * state = &CAST_OBJECT(struct state, node->state);
    state->output = object_alloc(chunk_type);
    CAST_OBJECT(double, state->output) = 0.0;
    state->status = STATUS_NEW;

    return node;
}

