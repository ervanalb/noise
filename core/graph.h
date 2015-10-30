#ifndef __CORE_GRAPH_H__
#define __CORE_GRAPH_H__

#include <stddef.h>

#include "noise.h"
#include "core/ntype.h"
#include "core/block.h"
#include "core/util.h"

struct nz_graph;

// This will be defined privately in the C file
// struct nz_node;
// struct nz_node {
//     char * node_id;
//     struct nz_block_info block_info;
//     struct nz_block block;
//     struct nz_node ** upstream_node_p_array;
//     struct nz_node ** downstream_node_p_array;
//     struct nz_node * next; // For linked list
// }

struct nz_graph {
    const struct nz_context * context_p;
    n_registered_typeclasses = 0;
    registered_typeclass_capacity = 16;
    registered_typeclasses = calloc(context_p->registered_typeclass_capacity, sizeof(struct nz_typeclass *));
    struct nz_node * graph_node_array;
}

// --
// Public interface

nz_rc nz_create_graph(const struct nz_context * context_p, struct nz_graph ** graph_pp);
void nz_destroy_graph(struct nz_graph * graph_p);

void nz_graph_add_block(struct nz_graph * graph_p, const char * id, const char * block, struct nz_block_info ** block_info_pp);
void nz_graph_del_block(struct nz_graph * graph_p, const char * id);
void nz_graph_connect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream);
void nz_graph_disconnect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream);
void nz_graph_inspect(const struct nz_graph * graph_p, const char * id, const char ** result);
void nz_graph_send(const struct nz_graph * graph_p, const char * id, const char * string);

#endif
