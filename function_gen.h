#include "block.h"

typedef struct {
	double out;
} function_gen_state_t;

error_t function_gen_state_alloc(block_info_pt block_info, state_pt* state);
void function_gen_state_free(block_info_pt block_info, state_pt state);
error_t function_gen_state_copy(block_info_pt block_info, state_pt dest, state_pt source);

error_t function_gen_pull(node_t* node, output_pt* output);
