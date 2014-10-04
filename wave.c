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

/*
    printf("pulling...\n");
    printf("node addr = %x\n",node);
    printf("node->input_node = %x\n",node->input_node);
    printf("node->input_pull = %x\n",node->input_pull);
    printf("node->input_pull* = %x\n", *((int **) (node->input_pull)));

//#define pull(N,I,O) ((N)->input_pull[(I)](&((N)->input_node[(I)]),(O)))
*/
	e=pull(node,0,(output_pt*)(&freq));
	if(e != SUCCESS) return e;

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

