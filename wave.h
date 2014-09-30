#include "node.h"

typedef struct {
	double t;
	double* chunk;
} wave_state_t;

void* wave_new();
void wave_del(void* state);
void* wave_pull(node_t* node);
