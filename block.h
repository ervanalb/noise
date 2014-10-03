#ifndef __NODE_H
#define __NODE_H
#include "error.h"

// Stuff relating to blocks
#define pull(N,I,O) ((N)->input_pull[(I)](&((N)->input_node[(I)]),(O)))

typedef void* state_pt;
typedef void* output_pt;
typedef void* block_info_pt;

struct node_t;

typedef error_t (*pull_fn_pt)(struct node_t* node, output_pt* output);

typedef error_t (*state_alloc_fn_pt)(block_info_pt type, state_pt * state);
typedef void (*state_free_fn_pt)(block_info_pt type, state_pt state);

typedef struct node_t
{
	struct node_t* input_node;
	pull_fn_pt* input_pull;
	state_pt state;
} node_t;


// Stuff relating to types
typedef void* type_info_pt;
typedef error_t (*output_alloc_fn_pt)(type_info_pt type, output_pt * output);
typedef error_t (*output_free_fn_pt)(type_info_pt type, output_pt output);
typedef error_t (*output_copy_fn_pt)(type_info_pt type, output_pt dest, output_pt src);
#endif
