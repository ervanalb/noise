#ifndef __BLOCK_H
#define __BLOCK_H

#include "error.h"
#include "typefns.h"
#include <stddef.h>

// Stuff relating to blocks
#define pull(N,I,O) ( ((N)->input_pull[(I)]) ? ((N)->input_pull[(I)]((N)->input_node[(I)],(O))) : ((*(O)=0), 0) )

typedef void* state_pt;
typedef void* output_pt;
typedef void* block_info_pt;

struct node_t;

typedef error_t (*pull_fn_pt)(struct node_t* node, output_pt* output);
typedef struct node_t * (*block_create_fn_pt)(block_info_pt block_info);
typedef void (*block_destroy_fn_pt)(struct node_t * node);

// Block instances are nodes

typedef struct node_t
{
    block_destroy_fn_pt destroy;

	object_t * state;

    size_t n_inputs;
	struct node_t** input_node;
	pull_fn_pt* input_pull;
    // * input_types;
    // const char ** input_names;

    size_t n_outputs;
    pull_fn_pt * output_pull;
    // * output_types;
    // const char ** output_names;
} node_t;

void generic_block_destroy(node_t * node);

#endif
