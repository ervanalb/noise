#include "block.h"

typedef struct {
    int length;
    double* sample;
} sample_info_t;

typedef struct {
	int t;
    int length;
    double* sample;
	double* chunk;
} sample_state_t;

error_t sample_state_alloc(block_info_pt block_info, state_pt* state);
void sample_state_free(block_info_pt block_info, state_pt state);
error_t sample_state_copy(block_info_pt block_info, state_pt dest, state_pt source);

error_t sample_pull(node_t* node, output_pt* output);
