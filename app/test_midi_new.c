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

    const char * err_line = NULL;

#define LINESTRING3(x) #x
#define LINESTRING2(x) LINESTRING3(x)
#define LINESTRING LINESTRING2(__LINE__)

#define ADD_BLOCK(name, block) rc = nz_graph_add_block(graph, name, block); if (rc != NZ_SUCCESS) { err_line = LINESTRING ", block: "#name " " #block; goto err; }
#define CONNECT(from, from_port, to, to_port) rc = nz_graph_connect(graph, from, from_port, to, to_port); if (rc != NZ_SUCCESS) { err_line = LINESTRING ", connect: " #from " " #from_port " " #to " " #to_port; goto err; }

    ADD_BLOCK("time", "accumulator");
    ADD_BLOCK("delta_time", "constant(real,0.0116)");
    //ADD_BLOCK("delta_time", "constant(real,0.0135)");
    ADD_BLOCK("time_tee", "tee(4,real)");
    ADD_BLOCK("n_beats_in_melody", "constant(real,32.0)");
    ADD_BLOCK("beat_in_melody", "mod");
    ADD_BLOCK("beat_in_melody_tee", "tee(2,real)");
    ADD_BLOCK("n_beats_in_bar", "constant(real,4.0)");
    ADD_BLOCK("beat_in_bar", "mod");
    ADD_BLOCK("beat_in_bar_tee", "tee(2,real)");

    ADD_BLOCK("unison_smf", "midireader(unison.midi)");
    ADD_BLOCK("melody", "midimelody");
    ADD_BLOCK("lpf_alpha", "constant(real,0.1)");
    ADD_BLOCK("lpf", "lpf");
    ADD_BLOCK("sound1", "wave(saw)");
    ADD_BLOCK("envelope", "envelope");

    ADD_BLOCK("drums_smf", "midireader(drums.midi)");
    ADD_BLOCK("drums", "mididrums");
    ADD_BLOCK("kick_drum", "drum(kick)");
    ADD_BLOCK("snare_drum", "drum(snare)");

    ADD_BLOCK("vol1", "constant(real,0.9)");
    ADD_BLOCK("vol2", "constant(real,0.5)");
    ADD_BLOCK("vol3", "constant(real,0.8)");
    ADD_BLOCK("vol4", "constant(real,0.5)");
    ADD_BLOCK("mix", "mixer(4)");
    ADD_BLOCK("compressor", "compressor(0.01)");
    ADD_BLOCK("soundcard", "wavfileout(new_unison.wav)");
    

    CONNECT("delta_time", "out", "time", "in");
    CONNECT("time", "out", "time_tee", "in");
    //CONNECT("time_tee", "main", "time_wye", "aux 1");
    CONNECT("time_tee", "main", "beat_in_bar", "a");
    CONNECT("n_beats_in_bar", "out", "beat_in_bar", "b");
    CONNECT("beat_in_bar", "out", "beat_in_bar_tee", "in");
    CONNECT("beat_in_bar_tee", "main", "drums_smf", "in");

    CONNECT("time_tee", "aux 1", "beat_in_melody", "a");
    CONNECT("n_beats_in_melody", "out", "beat_in_melody", "b");
    CONNECT("beat_in_melody", "out", "beat_in_melody_tee", "in");
    CONNECT("beat_in_melody_tee", "main", "unison_smf", "in");

    CONNECT("unison_smf", "out", "melody", "in");
    CONNECT("melody", "pitch out", "lpf", "in");
    CONNECT("lpf_alpha", "out", "lpf", "alpha");
    CONNECT("lpf", "out", "sound1", "freq");
    CONNECT("melody", "velocity out", "envelope", "velocity in");
    CONNECT("sound1", "out", "envelope", "chunk in");
    CONNECT("envelope", "out", "mix", "in 1");

    CONNECT("drums_smf", "out", "drums", "in");
    CONNECT("drums", "out 0", "kick_drum", "velocity");
    CONNECT("kick_drum", "out", "mix", "in 3");
    CONNECT("drums", "out 1", "snare_drum", "velocity");
    CONNECT("snare_drum", "out", "mix", "in 4");

    CONNECT("vol1", "out", "mix", "gain 1");
    CONNECT("vol2", "out", "mix", "gain 1");
    CONNECT("vol3", "out", "mix", "gain 3");
    CONNECT("vol4", "out", "mix", "gain 4");
    CONNECT("mix", "out", "compressor", "in");
    CONNECT("compressor", "out", "soundcard", "in");

    rc = nz_graph_block_handle(graph, "soundcard", &block_handle); if(rc != NZ_SUCCESS) goto err;
    //rc = pa_start(block_handle); if(rc != NZ_SUCCESS) goto err;
    int blocks_out = wavfileout_record(block_handle, 10);
    printf("Wrote %d blocks\n", blocks_out);

err:
    printf("Line: %s\n", err_line);
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

