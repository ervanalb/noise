#include "node.h"

typedef struct {
	double t;
	double* chunk;
} wave_state_t;

int wave_state_alloc(state_pt* state);
void wave_state_free(state_pt state);
int wave_state_copy(state_pt dest, state_pt source);
int wave_new(node_t * node, args_pt args);
void wave_del(node_t * node);
int wave_pull(node_t*, void **);
