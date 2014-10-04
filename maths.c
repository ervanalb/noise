#include "maths.h"
#include <math.h>
#include <stdlib.h>

error_t maths_state_alloc(block_info_pt block_info, state_pt* state)
{
	maths_state_t* maths_state;

	maths_state = malloc(sizeof(maths_state_t));

	*state = maths_state;

	if(!maths_state) return raise_error(ERR_MALLOC,"");

	return SUCCESS;
}

error_t plus_pull(node_t * node, output_pt * output)
{
	double* x;
	double* y;

	error_t e;

	e=pull(node,0,(output_pt*)(&x));
	if(e != SUCCESS) return e;

	e=pull(node,1,(output_pt*)(&y));
	if(e != SUCCESS) return e;

	maths_state_t* state = (maths_state_t*)(node->state);

	state->out = *x + *y;

	*output = ((void *) &(state->out));

	return SUCCESS;
}

error_t minus_pull(node_t * node, output_pt * output)
{
	double* x;
	double* y;

	error_t e;

	e=pull(node,0,(output_pt*)(&x));
	if(e != SUCCESS) return e;

	e=pull(node,1,(output_pt*)(&y));
	if(e != SUCCESS) return e;

	maths_state_t* state = (maths_state_t*)(node->state);

	state->out = *x - *y;

	*output = ((void *) &(state->out));

	return SUCCESS;
}

error_t multiply_pull(node_t * node, output_pt * output)
{
	double* x;
	double* y;

	error_t e;

	e=pull(node,0,(output_pt*)(&x));
	if(e != SUCCESS) return e;

	e=pull(node,1,(output_pt*)(&y));
	if(e != SUCCESS) return e;

	maths_state_t* state = (maths_state_t*)(node->state);

	state->out = *x * *y;

	*output = ((void *) &(state->out));

	return SUCCESS;
}

error_t divide_pull(node_t * node, output_pt * output)
{
	double* x;
	double* y;

	error_t e;

	e=pull(node,0,(output_pt*)(&x));
	if(e != SUCCESS) return e;

	e=pull(node,1,(output_pt*)(&y));
	if(e != SUCCESS) return e;

	maths_state_t* state = (maths_state_t*)(node->state);

	state->out = *x / *y;

	*output = ((void *) &(state->out));

	return SUCCESS;
}

