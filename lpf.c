#include "lpf.h"
#include "block.h"
#include "typefns.h"
#include "globals.h"
#include <stdlib.h>

error_t lpf_state_alloc(block_info_pt block_info, state_pt* state)
{
	lpf_state_t* lpf_state;
	lpf_state=malloc(sizeof(lpf_state_t));
	*state=lpf_state;
	if(!lpf_state) return raise_error(ERR_MALLOC,"");

	lpf_state->active = 0;

	return SUCCESS;
}

void lpf_state_free(block_info_pt block_info, state_pt state)
{
	free(state);
}

error_t lpf_pull(node_t * node, output_pt * output)
{
	error_t e;

	double* cur;
    double* alpha;

	lpf_state_t* lpf_state = (lpf_state_t*)(node->state);

	e=pull(node,0,(output_pt*)&cur);
	if(e != SUCCESS) return e;
	e=pull(node,1,(output_pt*)&alpha);
	if(e != SUCCESS) return e;

	if(!cur)
	{
		//lpf_state = 0;
		lpf_state->active = 0;
		*output = 0; // return NULL
		return SUCCESS;
	}
	if(!lpf_state->active)
	{
		lpf_state->active = 1;
		lpf_state->prev=*cur;
	}

	lpf_state->prev = lpf_state->prev + *alpha * (*cur - lpf_state->prev);

	*output = (void*)&(lpf_state->prev);

	return SUCCESS;
}

