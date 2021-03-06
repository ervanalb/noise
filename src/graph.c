#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "libnoise.h"

struct nz_node;

struct nz_node {
    char * node_id;
    struct nz_block_info block_info;
    const struct nz_blockclass * blockclass_p;
    struct nz_block block;
    struct nz_node ** upstream_node_p_array;
    struct nz_node ** downstream_node_p_array;
    struct nz_node * next; // For linked list
};

struct nz_graph {
    const struct nz_context * graph_context_p;
    struct nz_node * graph_node_head_p;
};

nz_rc nz_graph_create(const struct nz_context * context_p, struct nz_graph ** graph_pp) {
    struct nz_graph * graph_p;
    graph_p = calloc(1, sizeof(struct nz_graph));
    if(graph_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    // Create a dummy HEAD node so that linked-list operations have no special cases
    graph_p->graph_node_head_p = calloc(1, sizeof(struct nz_node));
    if(graph_p->graph_node_head_p == NULL) {
        free(graph_p);
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    graph_p->graph_context_p = context_p;

    *graph_pp = graph_p;
    
    return NZ_SUCCESS;
}

void nz_graph_destroy(struct nz_graph * graph_p) {
    struct nz_node * node_p = graph_p->graph_node_head_p;
    struct nz_node * next_node_p = node_p->next;

    free(node_p); // Free dummy node
    node_p = next_node_p;

    while(node_p != NULL) {
        next_node_p = node_p->next;

        nz_context_destroy_block(node_p->blockclass_p, &node_p->block, &node_p->block_info);

        free(node_p->node_id);
        free(node_p->upstream_node_p_array);
        free(node_p->downstream_node_p_array);

        free(node_p);

        node_p = next_node_p;
    }
}

nz_rc nz_graph_add_block(
        struct nz_graph * graph_p,
        const char * id,
        const char * block) {

    struct nz_node * node_p;
    nz_rc rc;

    // Seek to end of list
    // TODO check for duplicate names
    struct nz_node * ll_p;
    for(ll_p = graph_p->graph_node_head_p; ll_p->next != NULL; ll_p = ll_p->next);

    node_p = calloc(1, sizeof(struct nz_node));
    if(node_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    node_p->node_id = strdup(id);
    if(node_p->node_id == NULL) {
        free(node_p);
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    rc = nz_context_create_block(graph_p->graph_context_p, &node_p->blockclass_p, &node_p->block, &node_p->block_info, block);
    if(rc != NZ_SUCCESS) {
        free(node_p->node_id);
        free(node_p);
        return rc;
    }

    node_p->upstream_node_p_array = calloc(node_p->block_info.block_n_inputs, sizeof(struct nz_node *));
    if(node_p->upstream_node_p_array == NULL) {
        nz_context_destroy_block(node_p->blockclass_p, &node_p->block, &node_p->block_info);
        free(node_p->node_id);
        free(node_p);
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    node_p->downstream_node_p_array = calloc(node_p->block_info.block_n_outputs, sizeof(struct nz_node *));
    if(node_p->downstream_node_p_array == NULL) {
        nz_context_destroy_block(node_p->blockclass_p, &node_p->block, &node_p->block_info);
        free(node_p->upstream_node_p_array);
        free(node_p->node_id);
        free(node_p);
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }
 
    // Append node to linked list

    ll_p->next = node_p;

    return NZ_SUCCESS;
}

nz_rc nz_graph_del_block(struct nz_graph * graph_p, const char * id) {
    // TODO disconnect

    struct nz_node * prev_p;
    for(prev_p = graph_p->graph_node_head_p; prev_p->next != NULL && strcmp(prev_p->next->node_id, id) != 0; prev_p = prev_p->next);

    struct nz_node * node_p = prev_p->next;
    if(node_p == NULL) NZ_RETURN_ERR_MSG(NZ_NODE_NOT_FOUND, strdup(id));

    // Remove node from linked list
    prev_p->next = node_p->next;

    // Free
    nz_context_destroy_block(node_p->blockclass_p, &node_p->block, &node_p->block_info);
    free(node_p->node_id);
    free(node_p->upstream_node_p_array);
    free(node_p->downstream_node_p_array);
    free(node_p);

    return NZ_SUCCESS;
}

nz_rc nz_graph_connect(
        struct nz_graph * graph_p,
        const char * id_upstream,
        const char * output_port,
        const char * id_downstream,
        const char * input_port) {

    struct nz_node * upstream_p = NULL;
    struct nz_node * downstream_p = NULL;

    for(struct nz_node * node_p = graph_p->graph_node_head_p->next; node_p != NULL; node_p = node_p->next) {
        if(strcmp(node_p->node_id, id_upstream) == 0) upstream_p = node_p;
        else if(strcmp(node_p->node_id, id_downstream) == 0) downstream_p = node_p;
        if(upstream_p != NULL && downstream_p != NULL) break;
    }
    if(upstream_p == NULL) NZ_RETURN_ERR_MSG(NZ_NODE_NOT_FOUND, strdup(id_upstream));
    if(downstream_p == NULL) NZ_RETURN_ERR_MSG(NZ_NODE_NOT_FOUND, strdup(id_downstream));

    size_t output_index;
    for(output_index = 0; output_index < upstream_p->block_info.block_n_outputs; output_index++) {
        if(strcmp(upstream_p->block_info.block_output_port_array[output_index].block_port_name, output_port) == 0) break;
    }
    if(output_index == upstream_p->block_info.block_n_outputs) NZ_RETURN_ERR_MSG(NZ_PORT_NOT_FOUND, strdup(output_port));

    size_t input_index;
    for(input_index = 0; input_index < downstream_p->block_info.block_n_inputs; input_index++) {
        if(strcmp(downstream_p->block_info.block_input_port_array[input_index].block_port_name, input_port) == 0) break;
    }
    if(input_index == downstream_p->block_info.block_n_inputs) NZ_RETURN_ERR_MSG(NZ_PORT_NOT_FOUND, strdup(input_port));

    if(upstream_p->downstream_node_p_array[output_index] != NULL) NZ_RETURN_ERR(NZ_PORT_ALREADY_CONNECTED);
    if(downstream_p->upstream_node_p_array[input_index] != NULL) NZ_RETURN_ERR(NZ_PORT_ALREADY_CONNECTED);

    if(!nz_types_are_equal(upstream_p->block_info.block_output_port_array[output_index].block_port_typeclass_p,
                           upstream_p->block_info.block_output_port_array[output_index].block_port_type_p,
                           downstream_p->block_info.block_input_port_array[input_index].block_port_typeclass_p,
                           downstream_p->block_info.block_input_port_array[input_index].block_port_type_p)) {
        NZ_RETURN_ERR(NZ_TYPE_MISMATCH); // TODO print out types
    }

    upstream_p->downstream_node_p_array[output_index] = downstream_p;
    downstream_p->upstream_node_p_array[input_index] = upstream_p;
    downstream_p->block.block_upstream_pull_fn_p_array[input_index] = upstream_p->block_info.block_pull_fns[output_index];
    downstream_p->block.block_upstream_block_array[input_index] = upstream_p->block;
    downstream_p->block.block_upstream_output_index_array[input_index] = output_index;

    return NZ_SUCCESS;
}

nz_rc nz_graph_disconnect(
        struct nz_graph * graph_p,
        const char * id_upstream,
        const char * output_port,
        const char * id_downstream,
        const char * input_port) {

    struct nz_node * upstream_p = NULL;
    struct nz_node * downstream_p = NULL;

    for(struct nz_node * node_p = graph_p->graph_node_head_p->next; node_p != NULL; node_p = node_p->next) {
        if(strcmp(node_p->node_id, id_upstream) == 0) upstream_p = node_p;
        else if(strcmp(node_p->node_id, id_downstream) == 0) downstream_p = node_p;
        if(upstream_p != NULL && downstream_p != NULL) break;
    }
    if(upstream_p == NULL) NZ_RETURN_ERR_MSG(NZ_NODE_NOT_FOUND, strdup(id_upstream));
    if(downstream_p == NULL) NZ_RETURN_ERR_MSG(NZ_NODE_NOT_FOUND, strdup(id_downstream));

    size_t output_index;
    for(output_index = 0; output_index < upstream_p->block_info.block_n_outputs; output_index++) {
        if(strcmp(upstream_p->block_info.block_output_port_array[output_index].block_port_name, output_port) == 0) break;
    }
    if(output_index == upstream_p->block_info.block_n_outputs) NZ_RETURN_ERR_MSG(NZ_PORT_NOT_FOUND, strdup(output_port));

    size_t input_index;
    for(input_index = 0; input_index < downstream_p->block_info.block_n_inputs; input_index++) {
        if(strcmp(downstream_p->block_info.block_input_port_array[input_index].block_port_name, input_port) == 0) break;
    }
    if(input_index == downstream_p->block_info.block_n_inputs) NZ_RETURN_ERR_MSG(NZ_PORT_NOT_FOUND, strdup(input_port));

    if(upstream_p->downstream_node_p_array[output_index] != downstream_p ||
       downstream_p->upstream_node_p_array[input_index] != upstream_p) NZ_RETURN_ERR(NZ_PORTS_NOT_CONNECTED);

    upstream_p->downstream_node_p_array[output_index] = NULL;
    downstream_p->upstream_node_p_array[input_index] = NULL;
    nz_block_clear_upstream(&downstream_p->block, input_index);

    return NZ_SUCCESS;
}

nz_rc nz_graph_block_info(struct nz_graph * graph_p, const char * id, struct nz_block_info ** info_pp) {
    struct nz_node * node_p;
    for(node_p = graph_p->graph_node_head_p->next; node_p != NULL && strcmp(node_p->node_id, id) != 0; node_p = node_p->next);
    if(node_p == NULL) NZ_RETURN_ERR_MSG(NZ_NODE_NOT_FOUND, strdup(id));
    *info_pp = &node_p->block_info;
    return NZ_SUCCESS;
}

nz_rc nz_graph_block_handle(struct nz_graph * graph_p, const char * id, struct nz_block ** block_pp) {
    struct nz_node * node_p;
    for(node_p = graph_p->graph_node_head_p->next; node_p != NULL && strcmp(node_p->node_id, id) != 0; node_p = node_p->next);
    if(node_p == NULL) NZ_RETURN_ERR_MSG(NZ_NODE_NOT_FOUND, strdup(id));
    *block_pp = &node_p->block;
    return NZ_SUCCESS;
}

nz_rc nz_graph_export_dot(struct nz_graph * graph_p, const char * filename) {
    FILE * file = fopen(filename, "w");
    if (file == NULL) NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strerror(errno));

    fprintf(file, "digraph noise {\n\n");

    // Draw each of the nodes
    for(struct nz_node * node_p = graph_p->graph_node_head_p; node_p != NULL; node_p = node_p->next) {
        fprintf(file, "subgraph cluster_node_%p {\n", node_p);
        fprintf(file, "    label=\"%s\";\n", node_p->node_id);
        fprintf(file, "    color=black;\n");
        fprintf(file, "    rankdir=TD;\n\n");
        fprintf(file, "    {\n");
        fprintf(file, "        rank=same;\n");
        fprintf(file, "        node_outputs_%p [style=invis];\n", node_p);

        for(size_t output_index = 0; output_index < node_p->block_info.block_n_outputs; output_index++) {
            const struct nz_port_info * port = &node_p->block_info.block_output_port_array[output_index];
            fprintf(file, "        port_%p [label=\"%s\", color=blue];\n", port, port->block_port_name);
            // XXX This is a hack to prevent the midi_drums block from getting too crazy
            if (output_index > 8) break;
        }

        fprintf(file, "    }\n");
        fprintf(file, "    {\n");
        fprintf(file, "        rank=same;\n");
        fprintf(file, "        node_inputs_%p [style=invis];\n", node_p);

        for(size_t input_index = 0; input_index < node_p->block_info.block_n_inputs; input_index++) {
            const struct nz_port_info * port = &node_p->block_info.block_input_port_array[input_index];
            fprintf(file, "        port_%p [label=\"%s\", color=green];\n", port, port->block_port_name);
        }
        fprintf(file, "    }\n");
        fprintf(file, "    node_inputs_%p -> node_outputs_%p [style=invis];\n", node_p, node_p);

        fprintf(file, "}\n\n");
    }

    // Make all the connections
    for(struct nz_node * node_p = graph_p->graph_node_head_p; node_p != NULL; node_p = node_p->next) {
        for(size_t input_index = 0; input_index < node_p->block_info.block_n_inputs; input_index++) {
            const struct nz_port_info * input_port = &node_p->block_info.block_input_port_array[input_index];
            size_t output_index = node_p->block.block_upstream_output_index_array[input_index];
            if (node_p->upstream_node_p_array[input_index] == NULL) continue;
            const struct nz_port_info * output_port = &node_p->upstream_node_p_array[input_index]->block_info.block_output_port_array[output_index];
            const char * color = "black";
            fprintf(file, "port_%p -> port_%p [color=%s];\n", output_port, input_port, color);
        }
        fprintf(file, "\n");
    }

    fprintf(file, "\n\n}");
    fclose(file);
    return 0;
}
