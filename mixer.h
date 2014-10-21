#include "block.h"

typedef struct {
    int num_channels;
    double* output;
} mixer_state_t;

error_t mixer_state_alloc(block_info_pt block_info, state_pt* state);
void mixer_state_free(block_info_pt block_info, state_pt state);
error_t mixer_state_copy(block_info_pt block_info, state_pt dest, state_pt source);

error_t mixer_pull(node_t* node, output_pt* output);
