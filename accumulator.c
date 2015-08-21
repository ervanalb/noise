#include <stdlib.h>
#include "globals.h"
#include "error.h"
#include "block.h"

typedef struct {
	double t;
} accumulator_state_t;

static error_t accumulator_pull(node_t * node, output_pt * output)
{
	accumulator_state_t* accumulator_state = (accumulator_state_t *) &node->state->object_data;

	double* dt;

	pull(node,0,(output_pt*)&dt);

	accumulator_state->t += *dt;

	*output = &(accumulator_state->t);

	return SUCCESS;
}

node_t * accumulator_create(block_info_pt block_info)
{
    static type_t * state_type = NULL;

    if (state_type == NULL)
        state_type = make_simple_type(sizeof(accumulator_state_t));

    if (state_type == NULL) return NULL;

    node_t * node = calloc(1, sizeof(node_t));
    if (node == NULL) return NULL;

    node->state = object_alloc(state_type);
    if (node->state == NULL) return free(node), NULL;

	accumulator_state_t* state = (accumulator_state_t *) &node->state->object_data;
    state->t = 0.0;

    static pull_fn_pt output_pull[] = {&accumulator_pull};
    node->output_pull = output_pull;
    node->n_outputs = 1;

    node->destroy = &generic_block_destroy;
    return node;
}

