#include "block.h"

typedef struct {
	double out;
} maths_state_t;

error_t maths_state_alloc(block_info_pt block_info, state_pt* state);
void maths_state_free(block_info_pt block_info, state_pt state);
error_t maths_state_copy(block_info_pt block_info, state_pt dest, state_pt source);

error_t plus_pull(node_t* node, output_pt* output);
error_t minus_pull(node_t* node, output_pt* output);
error_t multiply_pull(node_t* node, output_pt* output);
error_t divide_pull(node_t* node, output_pt* output);
