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
//
// struct nz_graph {
//     const struct nz_context * context_p;
//     struct nz_graph * graph_node_head;
/// }

// --
// Public interface

nz_rc nz_create_graph(const struct nz_context * context_p, struct nz_graph ** graph_pp);
void nz_destroy_graph(struct nz_graph * graph_p);

nz_rc nz_graph_add_block(struct nz_graph * graph_p, const char * id, const char * block, struct nz_block_info ** block_info_pp);
nz_rc nz_graph_del_block(struct nz_graph * graph_p, const char * id);
nz_rc nz_graph_connect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream);
nz_rc nz_graph_disconnect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream);
nz_rc nz_graph_inspect(const struct nz_graph * graph_p, const char * id, const char ** result);
nz_rc nz_graph_send(const struct nz_graph * graph_p, const char * id, const char * string);

#endif
