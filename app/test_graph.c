#include "noise.h"
#include <stdio.h>
#include <stdlib.h>

const size_t nz_chunk_size = 128;

// TODO move this somewhere sane
nz_rc debug_pull(struct nz_block *);

nz_rc run()
{
    nz_rc rc = NZ_SUCCESS;
    struct nz_context * context;
    struct nz_graph * graph;
    struct nz_block * block_handle1;
    struct nz_block * block_handle2;

    if((rc = nz_context_create(&context)) == NZ_SUCCESS)
    {
        // Create a graph
        if((rc = nz_graph_create(context, &graph)) == NZ_SUCCESS)
        {
            rc = nz_graph_add_block(graph, "1", "constant(real, 20)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "2", "accumulator", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "3", "tee(2, real)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "4", "debug(real)", &block_handle1); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "5", "debug(real)", &block_handle2); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "1", "out", "2", "in"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "2", "out", "3", "in"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "3", "main", "4", "in"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "3", "aux 1", "5", "in"); if(rc != NZ_SUCCESS) goto err;
            rc = debug_pull(block_handle1); if(rc != NZ_SUCCESS) goto err;
            rc = debug_pull(block_handle2); if(rc != NZ_SUCCESS) goto err;
            rc = debug_pull(block_handle2); if(rc != NZ_SUCCESS) goto err;
            rc = debug_pull(block_handle1); if(rc != NZ_SUCCESS) goto err;
            rc = debug_pull(block_handle2); if(rc != NZ_SUCCESS) goto err;
            err:
            nz_graph_destroy(graph);
        }
        nz_context_destroy(context);
    }
    return rc;
}

int main()
{
    nz_rc rc = run();
    if(rc != NZ_SUCCESS)
    {
        fprintf(stderr, "Noise error %d: %s\n", rc, nz_error_rc_str(rc));
        fprintf(stderr, "File: %s line %d\n", nz_error_file, nz_error_line);
        if(nz_error_string) {
            fprintf(stderr, "%s\n", nz_error_string);
            free(nz_error_string);
        }
        return 1;
    }
    return 0;
}

