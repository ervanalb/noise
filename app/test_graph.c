#include "noise.h"
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include "blocks/blocks.h"

const size_t nz_chunk_size = 128;

nz_rc run()
{
    nz_rc rc = NZ_SUCCESS;
    struct nz_context * context;
    struct nz_graph * graph;
    struct nz_block * block_handle;

    Pa_Initialize();

    if((rc = nz_context_create(&context)) == NZ_SUCCESS)
    {
        // Create a graph
        if((rc = nz_graph_create(context, &graph)) == NZ_SUCCESS)
        {
            rc = nz_graph_add_block(graph, "1", "constant(chunk,{0.5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0})", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "2", "pa", &block_handle); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "1", "out", "2", "in"); if(rc != NZ_SUCCESS) goto err;
            rc = pa_start(block_handle); if(rc != NZ_SUCCESS) goto err;
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

