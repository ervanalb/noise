#include "accumulator.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "node.h"
#include "globals.h"
#include "error.h"

int 
accumulator_new(node_t * node)
{
	accumulator_state_t* state = malloc(sizeof(accumulator_state_t));
	if(!state) return raise_error(ERR_MALLOC, node, "malloc failed");

	state->t = 0;

	node->state = state;

	return 0;
}

int 
accumulator_del(node_t * node)
{
	free(node->state);
	return 0;
}

int 
accumulator_pull(node_t * node, void ** output)
{
	double* dt=node->input_pull[0](&(node->input[0]));
	int* reset=node->input_pull[1](&(node->input[1]));

	accumulator_state_t* state = (accumulator_state_t*)(node->state);

	if(*reset)
	{
		state->t = 0;
	}
	else
	{
		state->t += *dt;
	}

	*output = ((void *) &(state->t));

	return 0;
}

