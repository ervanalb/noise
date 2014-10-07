#include "sequencer.h"
#include <math.h>
#include <stdlib.h>

error_t sequencer_state_alloc(block_info_pt block_info, state_pt* state)
{
    sequencer_info_t* sequencer_info = (sequencer_info_t*)block_info;
    array_info_t* array_info = sequencer_info->array_info;

    sequencer_state_t* sequencer_state;

    sequencer_state = malloc(sizeof(sequencer_state_t));

    *state = sequencer_state;

    if(!sequencer_state) return raise_error(ERR_MALLOC,"");

    sequencer_state->length = array_info->length;

    return object_alloc(&array_info->element,&sequencer_state->object_state);
}

void sequencer_state_free(block_info_pt block_info, state_pt state)
{
    sequencer_state_t* sequencer_state = (sequencer_state_t*)state;
    sequencer_info_t* sequencer_info = (sequencer_info_t*)block_info;
    array_info_t* array_info = sequencer_info->array_info;

    object_free(&array_info->element,&sequencer_state->object_state);

    free(sequencer_state);
}

error_t sequencer_pull(node_t * node, output_pt * output)
{
    double* t;
    output_array_t* array;
    error_t e;

    e=pull(node,0,(output_pt*)(&t));
    if(e != SUCCESS) return e;
    e=pull(node,1,(output_pt*)(&array));
    if(e != SUCCESS) return e;

    sequencer_state_t* sequencer_state = (sequencer_state_t*)(node->state);

    output_pt element;
    element = array[((int)(*t)) % sequencer_state->length];

    if(element)
    {
        object_copy(&sequencer_state->object_state,element);
        *output = sequencer_state->object_state.object;
    }
    else
    {
        *output = 0;
    }

    return SUCCESS;
}

