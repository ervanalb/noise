#include "wave.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"
#include "globals.h"

error_t wave_state_alloc(type_info_pt type_info, state_pt* state)
{
	wave_state_t* wave_state;

	wave_state = malloc(sizeof(wave_state_t));

	*state = wave_state;

	if(!wave_state) return raise_error(ERR_MALLOC,"");

	wave_state->t = 0;
	wave_state->chunk = malloc(sizeof(double)*global_chunk_size);

	if(!wave_state->chunk) return raise_error(ERR_MALLOC,"");

	return SUCCESS;
}

void wave_state_free(type_info_pt type_info, state_pt state)
{
	free(((wave_state_t*)state)->chunk);
	free(state);
}

error_t wave_state_copy(type_info_pt type_info, state_pt dest, state_pt source)
{
	((wave_state_t*)dest)->t = ((wave_state_t*)source)->t;
	memcpy(((wave_state_t*)dest)->chunk,((wave_state_t*)source)->chunk,sizeof(double)*global_chunk_size);
	return SUCCESS;
}

static double f(double t)
{
	return sin(t*2*M_PI);
}

error_t wave_pull(node_t * node, output_pt * output)
{
	double* freq=pull(node,0);

	wave_state_t* state = (wave_state_t*)(node->state);

	int i;

	for(i=0;i<global_chunk_size;i++)
	{
		state->chunk[i] = f(state->t);
		state->t += *freq/global_frame_rate;
	}

	*output = ((void *) (state->chunk));

	return SUCCESS;
}

