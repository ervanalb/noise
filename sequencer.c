#include "sequencer.h"
#include <math.h>
#include <stdlib.h>

error_t sequencer_state_alloc(block_info_pt block_info, state_pt* state)
{
	sequencer_state_t* sequencer_state;

	sequencer_state = malloc(sizeof(sequencer_state_t));

	*state = sequencer_state;

	if(!sequencer_state) return raise_error(ERR_MALLOC,"");

	return SUCCESS;
}

void sequencer_state_free(block_info_pt block_info, state_pt state)
{
	free(state);
}

error_t sequencer_pull(node_t * node, output_pt * output)
{
	double* t;
	sequence_t* seq;
	error_t e;

	e=pull(node,0,(output_pt*)(&t));
	if(e != SUCCESS) return e;
	e=pull(node,1,(output_pt*)(&seq));
	if(e != SUCCESS) return e;

	sequencer_state_t* state = (sequencer_state_t*)(node->state);

	double* note;
    note = seq->array[((int)(*t)) % seq->length];

	if(note)
	{
		state->out = *note;
		*output = ((void *) &(state->out));
	}
	else
	{
		*output = 0;
	}

	return SUCCESS;
}

