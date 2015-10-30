#include "core/graph.h"

struct nz_node;

struct nz_node {
    char * node_id;
    struct nz_block_info block_info;
    struct nz_block block;
    struct nz_node ** upstream_node_p_array;
    struct nz_node ** downstream_node_p_array;
    struct nz_node * next; // For linked list
};

struct nz_graph {
    const struct nz_context * context_p;
    struct nz_graph * graph_node_head;
};

nz_rc nz_create_graph(const struct nz_context * context_p, struct nz_graph ** graph_pp) {
    return NZ_NOT_IMPLEMENTED;
}

void nz_destroy_graph(struct nz_graph * graph_p) {
}

nz_rc nz_graph_add_block(struct nz_graph * graph_p, const char * id, const char * block, struct nz_block_info ** block_info_pp, struct nz_block ** block_pp) {
    return NZ_NOT_IMPLEMENTED;
}

nz_rc nz_graph_del_block(struct nz_graph * graph_p, const char * id) {
    return NZ_NOT_IMPLEMENTED;
}

nz_rc nz_graph_connect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream) {
    return NZ_NOT_IMPLEMENTED;
}

nz_rc nz_graph_disconnect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream) {
    return NZ_NOT_IMPLEMENTED;
}
