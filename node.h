#ifndef __NODE_H
#define __NODE_H

struct _node_t;

typedef void* (*pull_fn_t)(struct _node_t* node);

typedef struct _node_t
{
	struct _node_t* input;
	pull_fn_t* input_pull;
	void* state;
} node_t;

#endif
