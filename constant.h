#include "block.h"

error_t constant_state_alloc(block_info_pt block_info, state_pt* state);
void constant_state_free(block_info_pt block_info, state_pt state);
error_t constant_pull(node_t * node, output_pt * output);
