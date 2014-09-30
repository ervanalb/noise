#include "wave.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "node.h"
#include "globals.h"
#include "error.h"

int wave_new(node_t * node)
{
	wave_state_t* state = malloc(sizeof(wave_state_t));
    if(!state) return raise_error(ERR_MALLOC, node, "malloc failed");

	state->t = 0;
	state->chunk = malloc(sizeof(double)*global_chunk_size);
    if(!state->chunk) return raise_error(ERR_MALLOC, node, "malloc failed");

    node->state = state;

	return 0;
}

int wave_del(node_t * node)
{
	free(((wave_state_t*)node->state)->chunk);
	free(node->state);
    return 0;
}

static double f(double t)
{
	return sin(t*2*M_PI);
}

int wave_pull(node_t * node, void ** output)
{
	double* freq=node->input_pull[0](&(node->input[0]));

	wave_state_t* state = (wave_state_t*)(node->state);

	int i;

	for(i=0;i<global_chunk_size;i++)
	{
		state->chunk[i] = f(state->t);
		state->t += *freq/global_frame_rate;
	}

	*output = ((void *) (state->chunk));

    return 0;
}

