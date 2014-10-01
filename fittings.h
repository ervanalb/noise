#include "node.h"

typedef struct
{
	state_pt up_state;
} tee_state_t;

typedef struct
{
	alloc_fn_pt up_alloc_fn;
	copy_fn_pt up_copy_fn;
	free_fn_pt up_free_fn;
} tee_type_t;

int tee_state_alloc(state_pt * state);
int tee_state_copy(state_pt dest, state_pt src);
void tee_state_free(state_pt state);
int tee_new(node_t * node, args_pt args);
void tee_del(node_t * node);
int tee_pull_main(node_t * node, output_pt* output);
int tee_pull_aux(node_t * node, output_pt* output);


