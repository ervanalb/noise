#include "wave.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "typefns.h"
#include "globals.h"

error_t wave_state_alloc(block_info_pt block_info, state_pt* state)
{
	wave_state_t* wave_state;

	wave_state = malloc(sizeof(wave_state_t));

	*state = wave_state;

	if(!wave_state) return raise_error(ERR_MALLOC,"");

	wave_state->t = 0;

	error_t e;

	e=chunk_alloc(0,(output_pt*)(&(wave_state->chunk)));
	if(e != SUCCESS) return e;

	return SUCCESS;
}

void wave_state_free(block_info_pt block_info, state_pt state)
{
	chunk_free(0,((wave_state_t*)state)->chunk);
	free(state);
}

static double f(double t)
{
	return sin(t*2*M_PI);
}

error_t wave_pull(node_t * node, output_pt * output)
{
	double* freq;
	error_t e;

	e=pull(node,0,(output_pt*)(&freq));
	if(e != SUCCESS) return e;

	wave_state_t* state = (wave_state_t*)(node->state);

	int i;

	for(i=0;i<global_chunk_size;i++)
	{
		state->chunk[i] = f(state->t);
		if(freq)
		{
			state->t += *freq/global_frame_rate;
		}
	}

	*output = ((void *) (state->chunk));

	return SUCCESS;
}

