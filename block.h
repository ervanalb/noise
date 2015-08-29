#ifndef __BLOCK_H
#define __BLOCK_H

#include <stddef.h>

#include "error.h"
#include "ntypes.h"

struct node;
struct port;

typedef int (*port_pull_fn_pt)(struct port * port);
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
    const struct port * inport_connection;
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
int port_connect(const struct port * output, struct inport * input);

static inline object_t * port_pull(struct port * port) {
    if (port == NULL)
        return (errno = EINVAL, NULL);

    assert(port->port_pull);
    int rc = port->port_pull(port);

    if (rc != 0)
        return NULL;

    return port->port_value;
}

#endif
