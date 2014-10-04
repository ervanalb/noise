#include "function_gen.h"
#include <math.h>
#include <stdlib.h>

error_t function_gen_state_alloc(block_info_pt block_info, state_pt* state)
{
	function_gen_state_t* function_gen_state;

	function_gen_state = malloc(sizeof(function_gen_state_t));

	*state = function_gen_state;

	if(!function_gen_state) return raise_error(ERR_MALLOC,"");

	return SUCCESS;
}

void function_gen_state_free(block_info_pt block_info, state_pt state)
{
	free(state);
}

static double f(double t)
{
	return sin(t*2*M_PI);
}

error_t function_gen_pull(node_t * node, output_pt * output)
{
	double* t;
	error_t e;

	e=pull(node,0,(output_pt*)(&t));
	if(e != SUCCESS) return e;

	function_gen_state_t* state = (function_gen_state_t*)(node->state);

	state->out = f(*t);

	*output = ((void *) &(state->out));

	return SUCCESS;
}

