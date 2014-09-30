#ifndef __NODE_H
#define __NODE_H

typedef void* args_t;
typedef void* state_t;
typedef void* output_t;

struct _node_t;

typedef output_t (*pull_fn_t)(struct _node_t* node);

typedef struct _node_t
{
	struct _node_t* input;
	pull_fn_t* input_pull;
	state_t state;
} node_t;


#endif
