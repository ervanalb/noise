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
    char * name;
};

struct node_input
{
    const type_t * type;
    char * name;
    struct endpoint * connected_input;
};

typedef struct node
{
    char * name;
    block_destroy_fn_pt destroy;

    object_t * state;

    size_t n_inputs;
    struct node_input * inputs;

    size_t n_outputs;
    struct endpoint outputs[];
} node_t;

void node_destroy_generic(node_t * node);
node_t * node_alloc(size_t n_inputs, size_t n_outputs, const type_t * state_type);
node_t * node_dup(node_t * src); // XXX: Maybe we shouldn't use this? 

static inline void node_destroy(node_t * node) 
{
    node->destroy(node);
}

// Connect blocks & pull

//#define pull(N,I,O) ( ((N)->input_pull[(I)]) ? ((N)->input_pull[(I)]((N)->input_node[(I)],(O))) : ((*(O)=0), 0) )

#include <stdio.h>

static inline error_t node_pull(struct node * node, size_t index, object_t ** output)
{
    if (index >= node->n_inputs) 
        return ERR_INVALID;

    if (node->inputs[index].connected_input == NULL) {
        *output = NULL;
        return SUCCESS;
    } else {
        struct endpoint * inp = node->inputs[index].connected_input;
        //printf("pull: %s\n", inp->name);
        return inp->pull(inp->node, output);
    }
}

error_t node_connect(struct node * dst, size_t dst_idx, struct node * src, size_t src_idx);
#endif
