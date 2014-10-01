#include "node.h"

typedef struct {
	double t;
} accumulator_state_t;

error_t accumulator_state_alloc(type_info_pt type_info, state_pt* state);
void accumulator_state_free(type_info_pt type_info, state_pt state);
error_t accumulator_state_copy(type_info_pt type_info, state_pt dest, state_pt source);

error_t accumulator_pull(node_t* node, output_pt* output);
