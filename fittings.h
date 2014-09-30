#include "node.h"

typedef struct
{
	alloc_fn_pt alloc_fn;
	copy_fn_pt copy_fn;
	free_fn_pt free_fn;
	state_pt memory;
} tee_state_t;

typedef struct
{
	alloc_fn_pt alloc_fn;
	copy_fn_pt copy_fn;
	free_fn_pt free_fn;
} tee_args_t;

int tee_state_alloc(state_pt * state);
int tee_state_copy(state_pt dest, state_pt src);
void tee_state_free(state_pt state);
int tee_new(node_t * node, args_pt args);
void tee_del(node_t * node);
int tee_pull_main(node_t * node, output_pt* output);
int tee_pull_aux(node_t * node, output_pt* output);


