#include "accumulator.h"
#include <stdlib.h>
#include "globals.h"

error_t accumulator_state_alloc(block_info_pt block_info, state_pt* state)
{
	accumulator_state_t* accumulator_state;

	accumulator_state = malloc(sizeof(accumulator_state_t));

	*state = accumulator_state;

	if(!accumulator_state) return raise_error(ERR_MALLOC,"");

	accumulator_state->t = 0;

	return SUCCESS;
}

void accumulator_state_free(block_info_pt block_info, state_pt state)
{
	free(state);
}

error_t accumulator_state_copy(block_info_pt block_info, state_pt dest, state_pt source)
{
	((accumulator_state_t*)dest)->t = ((accumulator_state_t*)source)->t;
	return SUCCESS;
}

error_t accumulator_pull(node_t * node, output_pt * output)
{
	accumulator_state_t* accumulator_state = node->state;

	double* dt;

	pull(node,0,(output_pt*)&dt);

	accumulator_state->t += *dt;

	*output = &(accumulator_state->t);

	return SUCCESS;
}

