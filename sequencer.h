#include "block.h"
#include "typefns.h"

typedef struct {
	int length;
	object_state_t object_state;
} sequencer_state_t;

typedef struct {
	array_info_t* array_info;
} sequencer_info_t;

error_t sequencer_state_alloc(block_info_pt block_info, state_pt* state);
void sequencer_state_free(block_info_pt block_info, state_pt state);
error_t sequencer_state_copy(block_info_pt block_info, state_pt dest, state_pt source);

error_t sequencer_pull(node_t* node, output_pt* output);
