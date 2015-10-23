#ifndef __CORE_BLOCK_H__
#define __CORE_BLOCK_H__

#include <stddef.h>

#include "noise.h"
#include "core/ntype.h"
#include "core/util.h"

struct nz_node;
struct nz_port;

enum nz_pull_rc {
    NZ_PULL_RC_ERROR  = -1,
    NZ_PULL_RC_NULL   = 0,
    NZ_PULL_RC_OBJECT = 1,
};

// Block instances are nodes

struct nz_port {
    struct nz_node * port_node;
    enum nz_pull_rc (*port_pull)(struct nz_port * port);
    nz_obj_p port_value; 

    const struct nz_type * port_type;
    char * port_name;
};

struct nz_inport {
    const struct nz_type * inport_type;
    char * inport_name;
    struct nz_port * inport_connection;
};

struct nz_node {
    char * node_name;
    void (*node_term)(struct nz_node * node);

    void * node_state;

    size_t node_n_inputs;
    struct nz_inport * node_inputs;

    size_t node_n_outputs;
    struct nz_port * node_outputs;

    struct {
        int flag_can_copy:1;
        int flag_pure:1;
        //int flag_visited:1;
    } node_flags;
};

// Helper function to allocate arrays of input & output ports
int nz_node_alloc_ports(struct nz_node * node, size_t n_inputs, size_t n_outputs);
//struct nz_node * nz_node_dup(const struct nz_node * src);

void nz_node_term_generic(struct nz_node * node);
void nz_node_term_generic_objstate(struct nz_node * node);
void nz_node_free_ports(struct nz_node * node);

void nz_node_term(struct nz_node * node);

// Get the name of a node, caller owns the resulting pointer
char * nz_node_str(struct nz_node * node);

// Connect blocks & pull
int nz_port_connect(struct nz_inport * input, struct nz_port * nz_output);
int nz_node_connect(struct nz_node* input, size_t in_idx, struct nz_node * output, size_t out_idx);

// These two don't seem that important
#define NZ_NODE_INPUT(node, idx) ((node)->node_inputs[(idx)].inport_connection->port_value)
#define NZ_NODE_OUTPUT(node, idx) ((node)->node_outputs[(idx)].port_value)

// Useful sugar
#define NZ_NODE_PULL(node, idx) nz_port_pull((node)->node_inputs[(idx)].inport_connection)

nz_obj_p nz_port_pull(struct nz_port * port);
// Use the macro if you can
nz_obj_p nz_node_pull(struct nz_node * node, size_t idx);
#endif
