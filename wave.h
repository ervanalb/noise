#include "block.h"

typedef struct {
	double t;
	double* chunk;
} wave_state_t;

error_t wave_state_alloc(block_info_pt block_info, state_pt* state);
void wave_state_free(block_info_pt block_info, state_pt state);
error_t wave_state_copy(block_info_pt block_info, state_pt dest, state_pt source);

error_t wave_pull(node_t* node, output_pt* output);
