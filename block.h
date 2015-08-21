#ifndef __BLOCK_H
#define __BLOCK_H

#include "error.h"
#include "typefns.h"
#include <stddef.h>

// Stuff relating to blocks
typedef void* state_pt;
typedef void* output_pt;
typedef void* block_info_pt;

struct node;

typedef error_t (*pull_fn_pt)(struct node * node, object_t ** output);
typedef struct node_t * (*block_create_fn_pt)(block_info_pt block_info);
typedef void (*block_destroy_fn_pt)(struct node  * node);

// Block instances are nodes

typedef struct endpoint
{
    struct node * node;
    pull_fn_pt pull;
    type_t * type;
    const char * name;
} endpoint_t;

typedef struct node
{
    block_destroy_fn_pt destroy;

	object_t * state;

    size_t n_inputs;
    struct endpoint ** inputs;

    size_t n_outputs;
    struct endpoint outputs[];
} node_t;

void generic_block_destroy(node_t * node);

//#define pull(N,I,O) ( ((N)->input_pull[(I)]) ? ((N)->input_pull[(I)]((N)->input_node[(I)],(O))) : ((*(O)=0), 0) )

static inline void pull(struct node * node, size_t index, object_t ** output)
{
    if (node->inputs[index] == NULL) {
        *output = NULL;
    } else {
        node->inputs[index]->pull(node->inputs[index]->node, output);
    }
}

#endif
