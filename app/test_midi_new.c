#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <stddef.h>
#include <mcheck.h>

#include "noise.h"
#include "nzlib/std.h"

nz_rc run()
{
    nz_rc rc = NZ_SUCCESS;
    struct nz_context * context;
    struct nz_lib * lib;
    struct nz_graph * graph;
    struct nz_block * block_handle;

    //Pa_Initialize();

    if((rc = nz_context_create(&context)) != NZ_SUCCESS) goto fail_context;
    if((rc = nz_context_load_lib(context, "nzlib/nzstd.so", &lib)) != NZ_SUCCESS) goto fail_lib;

    // Create a graph
    if((rc = nz_graph_create(context, &graph)) != NZ_SUCCESS) goto fail_graph;

    rc = nz_graph_add_block(graph, "time", "accumulator"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "delta_time", "constant(real,0.0135)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "n_beats_in_bar", "constant(real,32.0)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "beat_in_bar", "mod"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "beat_in_bar_tee", "tee(2,real)"); if(rc != NZ_SUCCESS) goto err;

    rc = nz_graph_add_block(graph, "unison_smf", "midireader(unison.midi)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "melody", "midimelody"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "lpf_alpha", "constant(real,0.1)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "lpf", "lpf"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "sound1", "wave(saw)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "envelope", "envelope"); if(rc != NZ_SUCCESS) goto err;

    rc = nz_graph_add_block(graph, "drums_smf", "midireader(drums.midi)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "drums", "mididrums"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "kick_drum", "drum(kick)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "snare_drum", "drum(snare)"); if(rc != NZ_SUCCESS) goto err;

    rc = nz_graph_add_block(graph, "vol1", "constant(real,0.0)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "vol2", "constant(real,0.5)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "vol3", "constant(real,0.5)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "vol4", "constant(real,0.5)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "tee", "tee(4,real)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "mix", "mixer(4)"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_add_block(graph, "soundcard", "wavfileout(new_unison.wav)"); if(rc != NZ_SUCCESS) goto err;
    
    

    rc = nz_graph_connect(graph, "delta_time", "out", "time", "in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "time", "out", "beat_in_bar", "a"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "n_beats_in_bar", "out", "beat_in_bar", "b"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "beat_in_bar", "out", "beat_in_bar_tee", "in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "beat_in_bar_tee", "main", "unison_smf", "in"); if(rc != NZ_SUCCESS) goto err;

    rc = nz_graph_connect(graph, "unison_smf", "out", "melody", "in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "melody", "pitch out", "lpf", "in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "lpf_alpha", "out", "lpf", "alpha"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "lpf", "out", "sound1", "freq"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "melody", "velocity out", "envelope", "velocity in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "sound1", "out", "envelope", "chunk in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "envelope", "out", "mix", "in 1"); if(rc != NZ_SUCCESS) goto err;

    rc = nz_graph_connect(graph, "beat_in_bar_tee", "aux 1", "drums_smf", "in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "drums_smf", "out", "drums", "in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "drums", "out 0", "kick_drum", "velocity"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "kick_drum", "out", "mix", "in 3"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "drums", "out 1", "snare_drum", "velocity"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "snare_drum", "out", "mix", "in 4"); if(rc != NZ_SUCCESS) goto err;

    rc = nz_graph_connect(graph, "vol1", "out", "mix", "gain 1"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "vol2", "out", "mix", "gain 2"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "vol3", "out", "mix", "gain 3"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "vol4", "out", "mix", "gain 4"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_connect(graph, "mix", "out", "soundcard", "in"); if(rc != NZ_SUCCESS) goto err;
    rc = nz_graph_block_handle(graph, "soundcard", &block_handle); if(rc != NZ_SUCCESS) goto err;
    //rc = pa_start(block_handle); if(rc != NZ_SUCCESS) goto err;
    int blocks_out = wavfileout_record(block_handle, 10);
    printf("Wrote %d blocks\n", blocks_out);

err:
fail_graph:
    nz_graph_destroy(graph);
fail_lib:
    nz_context_unload_lib(context, lib);
fail_context:
    nz_context_destroy(context);
    return rc;
}

int main()
{
    mcheck(0);
    nz_rc rc = run();
    if(rc != NZ_SUCCESS)
    {
        fprintf(stderr, "Noise error %d: %s\n", rc, nz_error_rc_str(rc));
        fprintf(stderr, "File: %s line %d\n", nz_error_file, nz_error_line);
        if(nz_error_string) {
            fprintf(stderr, "%s\n", nz_error_string);
            nz_free_str(nz_error_string);
        }
        return 1;
    }
    return 0;
}

