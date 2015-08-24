#include "sample.h"
#include <stdlib.h>
#include <string.h>
#include "typefns.h"
#include "globals.h"

typedef struct {
	int t;
    int length;
    double* sample;
	double* chunk;
} sample_state_t;

error_t sample_state_alloc(block_info_pt block_info, state_pt* state)
{
    sample_info_t* sample_info;
	sample_state_t* sample_state;

    sample_info = block_info;

	sample_state = malloc(sizeof(sample_state_t));

	*state = sample_state;

	if(!sample_state) return raise_error(ERR_MALLOC,"");

	sample_state->t = 0;
    sample_state->length = sample_info->length;
    sample_state->sample = malloc(sizeof(double)*sample_info->length);
	if(!sample_state->sample) return raise_error(ERR_MALLOC,"");
    memcpy(sample_state->sample, sample_info->sample, sizeof(double)*sample_info->length);

	error_t e;

	e = chunk_alloc(0,(output_pt*)(&sample_state->chunk));
	if(e != SUCCESS) return e;

	return SUCCESS;
}

void sample_state_free(block_info_pt block_info, state_pt state)
{
	chunk_free(0,((sample_state_t*)state)->chunk);
	free(((sample_state_t*)state)->sample);
	free(state);
}

error_t sample_pull(node_t * node, output_pt * output)
{
    int* play;
	error_t e;

	e=pull(node,0,(output_pt*)(&play));
	if(e != SUCCESS) return e;

	sample_state_t* state = (sample_state_t*)(node->state);

	int i;

    if(!play)
    {
        state->t = 0;
        *output = 0;
        return SUCCESS;
    }

    for(i=0;i<global_chunk_size;i++)
    {
        if(*play && state->t < state->length)
        {
            state->chunk[i]=state->sample[state->t++];
        }
        else
        {
            state->chunk[i] = 0;
        }
    }

    if(!*play)
    {
        state->t = 0;
    }

	*output = ((void *) (state->chunk));

	return SUCCESS;
}

