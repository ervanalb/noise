#include "block.h"

typedef struct {
	double t;
} accumulator_state_t;

error_t accumulator_state_alloc(block_info_pt block_info, state_pt* state);
void accumulator_state_free(block_info_pt block_info, state_pt state);

error_t accumulator_pull(node_t* node, output_pt* output);
