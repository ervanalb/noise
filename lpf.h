#include "typefns.h"
#include "globals.h"

typedef struct {
	int active;
	double prev;
} lpf_state_t;

error_t lpf_state_alloc(block_info_pt block_info, state_pt* state);
void lpf_state_free(block_info_pt block_info, state_pt state);
error_t lpf_pull(node_t * node, output_pt * output);
