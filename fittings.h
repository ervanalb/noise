#include "block.h"

typedef struct
{
	int active;
	type_info_pt type_info;
	output_copy_fn_pt copy_fn;
	output_pt output;
} union_state_t;

typedef struct
{
	type_info_pt type_info;
	output_alloc_fn_pt alloc_fn;
	output_copy_fn_pt copy_fn;
	output_free_fn_pt free_fn;
} union_info_t;

error_t union_state_alloc(block_info_pt block_info, state_pt* state);
void union_state_free(block_info_pt block_info, state_pt state);
error_t union_pull(node_t * node, output_pt * output);
error_t tee_pull_aux(node_t* node, output_pt * output);
error_t wye_pull(node_t * node, output_pt * output);

