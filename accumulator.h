#include "node.h"

typedef struct {
	double t;
} accumulator_state_t;

int accumulator_new(node_t * node);
int accumulator_del(node_t * node);
int accumulator_pull(node_t*, void **);
