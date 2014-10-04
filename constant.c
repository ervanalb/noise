#include "constant.h"
#include <stdlib.h>

error_t constant_state_alloc(block_info_pt block_info, state_pt* state)
{
    *state = block_info;
    return SUCCESS;
}

void constant_state_free(block_info_pt block_info, state_pt state)
{

}

error_t constant_pull(node_t * node, output_pt * output)
{
	*output = node->state;
	return SUCCESS;
}
