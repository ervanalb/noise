#include "wave.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "node.h"
#include "globals.h"

void* wave_new()
{
	wave_state_t* state = malloc(sizeof(wave_state_t));

	state->t = 0;
	state->chunk = malloc(sizeof(double)*global_chunk_size);

	return state;
}

void wave_del(void* state)
{
	free(((wave_state_t*)state)->chunk);
	free(state);
}

static double f(double t)
{
	return sin(t*2*M_PI);
}

void* wave_pull(node_t* node)
{
	double* freq=node->input_pull[0](&(node->input[0]));

	wave_state_t* state = (wave_state_t*)(node->state);

	int i;

	for(i=0;i<global_chunk_size;i++)
	{
		state->chunk[i] = f(state->t);
		state->t += *freq/global_frame_rate;
	}

	return state->chunk;
}

