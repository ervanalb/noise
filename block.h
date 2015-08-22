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

struct endpoint
{
    struct node * node;
    pull_fn_pt pull;
    const type_t * type;
    const char * name;
};

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
node_t * allocate_node(size_t n_inputs, size_t n_outputs, type_t * state_type);

// Connect blocks & pull

//#define pull(N,I,O) ( ((N)->input_pull[(I)]) ? ((N)->input_pull[(I)]((N)->input_node[(I)],(O))) : ((*(O)=0), 0) )

static inline void pull(struct node * node, size_t index, object_t ** output)
{
    if (node->inputs[index] == NULL) {
        *output = NULL;
    } else {
        node->inputs[index]->pull(node->inputs[index]->node, output);
    }
}

error_t connect(struct node * dst, size_t dst_idx, struct node * src, size_t src_idx);
#endif
