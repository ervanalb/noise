#include "block.h"

typedef struct
{
	int active;
	object_state_t object_state;
} union_state_t;

typedef struct
{
	int active;
    int num_aux_inputs;
	object_state_t object_state;
} wye_state_t;

typedef struct
{
    int num_aux_inputs;
	object_info_t object_info;
} wye_info_t;

error_t union_state_alloc(block_info_pt block_info, state_pt* state);
void union_state_free(block_info_pt block_info, state_pt state);
error_t union_pull(node_t * node, output_pt * output);
error_t tee_pull_aux(node_t* node, output_pt * output);
error_t wye_pull(node_t * node, output_pt * output);

