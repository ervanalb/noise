#ifndef __BLOCK_H
#define __BLOCK_H

#include <stddef.h>

#include "error.h"
#include "ntypes.h"

struct node;
struct port;

enum pull_rc {
    PULL_RC_ERROR  = -1,
    PULL_RC_NULL   = 0,
    PULL_RC_OBJECT = 1,
};

typedef enum pull_rc (*port_pull_fn_pt)(struct port * port);
typedef void (*node_term_fn_pt)(struct node  * node);

// Block instances are nodes

struct port {
    struct node * port_node;
    port_pull_fn_pt port_pull;
    object_t * port_value; 

    const struct type * port_type;
    char * port_name;
};

struct inport {
    const struct type * inport_type;
    char * inport_name;
    struct port * inport_connection;
};

typedef struct node {
    char * node_name;
    node_term_fn_pt node_term;
    struct {
        int flag_can_copy:1;
    } node_flags;

    void * node_state;

    size_t node_n_inputs;
    struct inport * node_inputs;

    size_t node_n_outputs;
    struct port * node_outputs;
} node_t;

int node_alloc_connections(node_t * node, size_t n_inputs, size_t n_outputs);
node_t * node_dup(const node_t * src);

void node_term_generic(node_t * node);
void node_term_generic_objstate(node_t * node);
void node_free_connections(node_t * node);

static inline void node_term(node_t * node) {
    if (node->node_term) node->node_term(node);
}

// Connect blocks & pull
int port_connect(struct inport * input, struct port * output);
int node_connect(node_t * input, size_t in_idx, node_t * output, size_t out_idx);

// These two don't seem that important
#define NODE_INPUT(node, idx) ((node)->node_inputs[(idx)].inport_connection->port_value)
#define NODE_OUTPUT(node, idx) ((node)->node_outputs[(idx)].port_value)

// Useful sugar
#define NODE_PULL(node, idx) port_pull((node)->node_inputs[(idx)].inport_connection)

static inline object_t * port_pull(struct port * port) {
    if (port == NULL)
        return (errno = EINVAL, NULL);

    assert(port->port_pull);
    enum pull_rc rc = port->port_pull(port);

    switch(rc) {
        case PULL_RC_ERROR:
            // TODO: log errors or something?
            return NULL;
        case PULL_RC_NULL:
            return NULL;
        case PULL_RC_OBJECT:
            return port->port_value;
        default:
            assert(0);
    }
}

#endif
