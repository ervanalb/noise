#include "fittings.h"
#include <stdlib.h>
#include "node.h"
#include "error.h"

int tee_state_alloc(state_pt * state)
{
	tee_state_t* tee_state = malloc(sizeof(tee_state_t));
	if(!tee_state) return -1;

	*state = tee_state;

	return 0;
}

int tee_state_copy(state_pt dest, state_pt src)
{
	// fuck
}

void tee_state_free(state_pt state)
{
	free(state);
}

int 
tee_new(node_t * node, args_pt args)
{
	tee_state_t* state;
	int result = tee_state_alloc((state_pt*)(&state));
	if(result) return raise_error(ERR_MALLOC, node, "alloc failed");

	state->alloc_fn = ((tee_args_t*)args)->alloc_fn;
	state->copy_fn = ((tee_args_t*)args)->copy_fn;
	state->free_fn = ((tee_args_t*)args)->free_fn;

	result = (*(state->alloc_fn))(state->memory);
	if(result) return raise_error(ERR_MALLOC, node, "child alloc failed");

	node->state = state;

	return 0;
}

void
tee_del(node_t * node)
{
	tee_state_t* state = node->state;

	(*(state->free_fn))(state->memory);

	tee_state_free(state);
}

int 
tee_pull_main(node_t * node, output_pt* output)
{
	output_pt result=node->input_pull[0](&(node->input[0]));
	tee_state_t* state = node->state;

	(*(state->copy_fn))(state->memory,result);
	*output = state->memory;
}

int
tee_pull_aux(node_t * node, output_pt* output)
{
	*output = ((tee_state_t*)(node->state))->memory;
}

