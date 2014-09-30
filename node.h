#ifndef __NODE_H
#define __NODE_H

typedef void* args_pt;
typedef void* state_pt;
typedef void* output_pt;

struct node_t;

typedef output_pt (*pull_fn_pt)(struct node_t* node);

typedef int (*alloc_fn_pt)(state_pt * state);
typedef int (*copy_fn_pt)(state_pt dest, state_pt src);
typedef void (*free_fn_pt)(state_pt state);

typedef struct node_t
{
	struct node_t* input;
	pull_fn_pt* input_pull;
	state_pt state;
} node_t;


#endif
