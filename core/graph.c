#include "core/graph.h"

struct nz_node;

struct nz_node {
    const char * node_id;
    struct nz_block_info block_info;
    struct nz_block block;
    struct nz_node ** upstream_node_p_array;
    struct nz_node ** downstream_node_p_array;
    struct nz_node * next; // For linked list
};

struct nz_graph {
    const struct nz_context * graph_context;
    struct nz_graph * graph_node_head;
    struct nz_node * graph_node_list; // For linked list
};

static nz_rc node_create(
        const struct nz_context * context_p, 
        struct nz_graph * graph,
        struct nz_node ** node) {
    *node = calloc(1, sizeof(**node)); 
    if(*node == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    // Append to node linked list
    (*node)->next = graph->graph_node_list;
    graph->graph_node_list = *node;

    return NZ_SUCCESS;
}

nz_rc nz_graph_create(const struct nz_context * context, struct nz_graph ** graph) {
    *graph = calloc(1, sizeof(**graph));
    if(*graph == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    (*graph)->graph_context = context;
    return NZ_SUCCESS;
}

void nz_graph_destroy(struct nz_graph * graph_pp) {
    // NZ_NOT_IMPLEMENTED
}

nz_rc nz_graph_add_block(
        struct nz_graph * graph_p,
        const char * id,
        const char * block,
        struct nz_block_info ** block_info_pp,
        struct nz_block ** block_pp) {
    return NZ_NOT_IMPLEMENTED;
}

nz_rc nz_graph_del_block(struct nz_graph * graph_p, const char * id) {
    return NZ_NOT_IMPLEMENTED;
}

nz_rc nz_graph_connect(
        struct nz_graph * graph_p,
        const char * id_upstream,
        const char * port_upstream,
        const char * id_downstream,
        const char * port_downstream) {
    return NZ_NOT_IMPLEMENTED;
}

nz_rc nz_graph_disconnect(
        struct nz_graph * graph_p,
        const char * id_upstream,
        const char * port_upstream,
        const char * id_downstream,
        const char * port_downstream) {
    return NZ_NOT_IMPLEMENTED;
}
