#include "convolve.h"
#include "typefns.h"
#include "globals.h"
#include <math.h>
#include <stdlib.h>

error_t convolve_state_alloc(block_info_pt block_info, state_pt* state)
{
    int input_len = *(int*)block_info;
    int input_size = input_len*sizeof(double);

	convolve_state_t* convolve_state;

	convolve_state = malloc(sizeof(convolve_state_t));

	*state = convolve_state;

	if(!convolve_state) return raise_error(ERR_MALLOC,"");

    convolve_state->input_length = input_len;
    convolve_state->history_pos = 0;

    error_t e;
    e=simple_alloc((type_info_pt)&input_size,(output_pt*)&convolve_state->history);
    if(e != SUCCESS) return e;

    e=chunk_alloc((type_info_pt)0,(output_pt*)&convolve_state->output);
    if(e != SUCCESS) return e;

    int i;
    for(i=0;i<input_len;i++)
    {
        convolve_state->history[i]=0;
    }

	return SUCCESS;
}

void convolve_state_free(block_info_pt block_info, state_pt state)
{
    if(state)
    {
        int input_len = *(int*)block_info;
        int input_size = input_len*sizeof(double);

        convolve_state_t* convolve_state = state;
        simple_free((type_info_pt)&input_size,(output_pt)convolve_state->history);
        chunk_free((type_info_pt)0,(output_pt)convolve_state->output);
    	free(state);
    }
}

error_t convolve_pull(node_t * node, output_pt * output)
{
	double* chunk_in;
	double* input;

    error_t e;

	e=pull(node,0,(output_pt*)(&chunk_in));
	if(e != SUCCESS) return e;

	e=pull(node,1,(output_pt*)(&input));
	if(e != SUCCESS) return e;

	convolve_state_t* state = (convolve_state_t*)(node->state);

    int input_len = state->input_length;
    int history_pos = state->history_pos;
    double* history = state->history;
    int i,j;

    for(i=0;i<global_chunk_size;i++)
    {
        history[history_pos] = chunk_in[i];

        state->output[i]=0;
        for(j=0;j<input_len;j++)
        {
            state->output[i] += input[j] * history[(history_pos-j+input_len) % input_len];
        }

        history_pos = (history_pos + 1) % input_len;
    }

    state->history_pos = history_pos;

    *output = state->output;

	return SUCCESS;
}

