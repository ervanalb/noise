#include "fittings.h"
#include <stdlib.h>

error_t union_state_alloc(block_info_pt block_info, state_pt* state)
{
	union_state_t* union_state;
	object_info_t* object_info = (object_info_t*)block_info;

	union_state = malloc(sizeof(union_state_t));

	*state = union_state;

	if(!union_state) return raise_error(ERR_MALLOC,"");

	union_state->active=0;
	return object_alloc(object_info,&union_state->object_state);
}

void union_state_free(block_info_pt block_info, state_pt state)
{
	object_info_t* object_info = (object_info_t*)block_info;
	union_state_t* union_state = (union_state_t*)state;

	object_free(object_info,&union_state->object_state);
	free(state);
}

error_t union_pull(node_t * node, output_pt * output)
{
	error_t e;
	union_state_t* union_state = (union_state_t*)(node->state);

	output_pt result=0;

	e=pull(node,0,&result);
	if(e != SUCCESS) return e;

	if(!result)
	{
		union_state->active = 0;
		*output = 0;
		return SUCCESS;
	}

	union_state->active = 1;

	object_copy(&union_state->object_state,result);
	if(e != SUCCESS) return e;

	*output = union_state->object_state.object;
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
		*output = union_state->object_state.object;
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

	object_copy(&union_state->object_state,result);
	if(e != SUCCESS) return e;

	*output = union_state->object_state.object;
	return SUCCESS;
}

