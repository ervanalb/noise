#include "node.h"

typedef struct {
	double t;
	double* chunk;
} wave_state_t;

int wave_new(node_t * node);
int wave_del(node_t * node);
int wave_pull(node_t*, void **);
