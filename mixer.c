#include "mixer.h"
#include "block.h"
#include "typefns.h"
#include "globals.h"
#include <stdlib.h>

error_t mixer_state_alloc(block_info_pt block_info, state_pt* state)
{
    error_t e;

	mixer_state_t* mixer_state;
	mixer_state=malloc(sizeof(mixer_state_t));
	*state=mixer_state;
	if(!mixer_state) return raise_error(ERR_MALLOC,"");

    e=chunk_alloc((type_info_pt)0,(output_pt*)&mixer_state->output);
    if(e != SUCCESS) return e;

	mixer_state->num_channels = *((int*)block_info);

	return SUCCESS;
}

void mixer_state_free(block_info_pt block_info, state_pt state)
{
	free(state);
}

error_t mixer_pull(node_t * node, output_pt * output)
{
    error_t e;
	mixer_state_t* mixer_state = (mixer_state_t*)(node->state);

    int i,j;
    double* gain;
    double* chunk;

    for(j=0;j<global_chunk_size;j++)
    {
        mixer_state->output[j]=0;
    }

    for(i=0;i<mixer_state->num_channels;i++)
    {
        e=pull(node,2*i,(output_pt*)&chunk);
        if(e != SUCCESS) return e;
        e=pull(node,2*i+1,(output_pt*)&gain);
        if(e != SUCCESS) return e;

        if(!chunk || !gain) continue;

        for(j=0;j<global_chunk_size;j++)
        {
            mixer_state->output[j] += *gain * chunk[j];
        }
    }

	*output = (output_pt)(mixer_state->output);

	return SUCCESS;
}

