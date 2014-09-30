#include "wave.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "node.h"
#include "globals.h"
#include "error.h"

int wave_state_alloc(state_pt* state)
{
	wave_state_t* wave_state;

	wave_state = malloc(sizeof(wave_state_t));

	if(!wave_state) return -1;

	wave_state->t = 0;
	wave_state->chunk = malloc(sizeof(double)*global_chunk_size);
	if(!wave_state->chunk) return -1;

	*state = wave_state;

	return 0;
}

void wave_state_free(state_pt state)
{
	free(((wave_state_t*)state)->chunk);
	free(state);
}

int wave_state_copy(state_pt dest, state_pt source)
{
	((wave_state_t*)dest)->t = ((wave_state_t*)source)->t;
	memcpy(((wave_state_t*)dest)->chunk,((wave_state_t*)source)->chunk,sizeof(double)*global_chunk_size);
	return 0;
}

int 
wave_new(node_t * node, args_pt args)
{
	return wave_state_alloc(&node->state);
}

void wave_del(node_t * node)
{
	wave_state_free(node->state);
}

static double 
f(double t)
{
	return sin(t*2*M_PI);
}

int 
wave_pull(node_t * node, output_pt * output)
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

