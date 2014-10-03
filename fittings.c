#include "fittings.h"
#include <stdlib.h>

error_t union_state_alloc(block_info_pt block_info, state_pt* state)
{
	union_state_t* union_state;
	union_info_t* union_info = (union_info_t*)block_info;

	union_state = malloc(sizeof(union_state_t));

	*state = union_state;

	if(!union_state) return raise_error(ERR_MALLOC,"");

	union_state->active=0;
	union_state->type_info = union_info->type_info;
	union_state->copy_fn = union_info->copy_fn;

	return (*(union_info->alloc_fn))(union_info->type_info,(output_pt*)(&(union_state->output)));
}

void union_state_free(block_info_pt block_info, state_pt state)
{
	union_info_t* union_info = (union_info_t*)block_info;

	(*(union_info->free_fn))(union_info->type_info,((union_state_t*)state)->output);
	free(state);
}

error_t union_pull(node_t * node, output_pt * output)
{
	error_t e;
	union_state_t* union_state = (union_state_t*)(node->state);

	output_pt result=0;

	e=pull(node,0,result);
	if(e != SUCCESS) return e;

	if(!result)
	{
		union_state->active = 0;
		*output = 0;
		return SUCCESS;
	}

	union_state->active = 1;

	e=(*(union_state->copy_fn))(union_state->type_info,union_state->output,result);
	if(e != SUCCESS) return e;

	*output = union_state->output;
	return SUCCESS;
}

error_t tee_pull_aux(node_t* node, output_pt * output)
{
	union_state_t* union_state = (union_state_t*)(node->state);

	if(!union_state->active)
	{
		*output = 0;
	}
	else
	{
		*output = union_state->output;
	}
	return SUCCESS;
}

error_t wye_pull(node_t * node, output_pt * output)
{
	error_t e;

	union_state_t* union_state = (union_state_t*)(node->state);

	output_pt result=0, garbage=0;

	e=pull(node,0,result);
	if(e != SUCCESS) return e;
	
	e=pull(node,1,garbage);
	if(e != SUCCESS) return e;

	if(!result)
	{
		union_state->active = 0;
		*output = 0;
		return SUCCESS;
	}

	union_state->active = 1;

	e=(*(union_state->copy_fn))(union_state->type_info,union_state->output,result);
	if(e != SUCCESS) return e;

	*output = union_state->output;
	return SUCCESS;
}

