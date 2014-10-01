#ifndef __NODE_H
#define __NODE_H
#include "error.h"

#define pull(N,I) ((N)->input_pull[(I)](&((N)->input_node[(I)])))


typedef void* state_pt;
typedef void* output_pt;
typedef void* type_info_pt;

struct node_t;

typedef output_pt (*pull_fn_pt)(struct node_t* node);

typedef int (*alloc_fn_pt)(type_info_pt type, state_pt * state);
typedef int (*copy_fn_pt)(type_info_pt type, state_pt dest, state_pt src);
typedef void (*free_fn_pt)(type_info_pt type, state_pt state);

typedef struct node_t
{
	struct node_t* input_node;
	pull_fn_pt* input_pull;
	type_info_pt type_info;
	state_pt state;
} node_t;


#endif
