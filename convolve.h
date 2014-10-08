#include "block.h"

typedef struct {
    int input_length;
    int history_pos;
	double* history;
    double* output;
} convolve_state_t;

error_t convolve_state_alloc(block_info_pt block_info, state_pt* state);
void convolve_state_free(block_info_pt block_info, state_pt state);
error_t convolve_state_copy(block_info_pt block_info, state_pt dest, state_pt source);

error_t convolve_pull(node_t* node, output_pt* output);
