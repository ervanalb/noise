#include "noise.h"
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include "blocks/blocks.h"

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
            rc = nz_graph_add_block(graph, "sound1", "wave(saw)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "sound2", "wave(saw)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "freq1", "constant(real,440)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "freq2", "constant(real,554.4)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "vol", "constant(real,0.5)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "tee", "tee(2,real)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "mix", "mixer(2)", NULL); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_add_block(graph, "soundcard", "pa", &block_handle); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "freq1", "out", "sound1", "freq"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "freq2", "out", "sound2", "freq"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "sound1", "out", "mix", "in 1"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "sound2", "out", "mix", "in 2"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "vol", "out", "tee", "in"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "tee", "main", "mix", "gain 1"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "tee", "aux 1", "mix", "gain 2"); if(rc != NZ_SUCCESS) goto err;
            rc = nz_graph_connect(graph, "mix", "out", "soundcard", "in"); if(rc != NZ_SUCCESS) goto err;
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
            nz_error_string_free();
        }
        return 1;
    }
    return 0;
}

