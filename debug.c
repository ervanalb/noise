#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "error.h"
#include "block.h"
#include "typefns.h"
#include "blockdef.h"

#define N_INPUTS 1
#define N_OUTPUTS 0

node_t * debug_create()
{
    node_t * node = calloc(1, sizeof(node_t) + N_OUTPUTS * sizeof(struct endpoint));
    if (node == NULL) return NULL;

    node->n_inputs = N_INPUTS;
    node->inputs = calloc(N_INPUTS, sizeof(struct endpoint *));
    if (node->inputs == NULL) return (free(node), NULL);

    node->n_outputs = N_OUTPUTS;

    node->destroy = &generic_block_destroy;
    return node;
}

void run_debug(node_t * debug_block) 
{
    char inp;
    do {
        object_t * obj;

        pull(debug_block, 0, &obj);
        printf(" > %f", CAST_OBJECT(double, obj));
    } while(scanf("%c", &inp), inp != 'q');
}

