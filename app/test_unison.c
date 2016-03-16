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
    ADD_BLOCK("delta_time", "constant(real,0.0058)");
    //ADD_BLOCK("delta_time", "constant(real,0.0135)");
    ADD_BLOCK("time_tee", "tee(4,real)");
    ADD_BLOCK("time_wye", "wye(2,real)");
    ADD_BLOCK("ruler", "ruler(8)");
    /*
    ADD_BLOCK("n_beats_in_melody", "constant(real,32.0)");
    ADD_BLOCK("beat_in_melody", "mod");
    ADD_BLOCK("beat_in_melody_tee", "tee(2,real)");
    ADD_BLOCK("n_beats_in_bar", "constant(real,4.0)");
    ADD_BLOCK("beat_in_bar", "mod");
    ADD_BLOCK("beat_in_bar_tee", "tee(2,real)");
    */

    ADD_BLOCK("gate", "gate(real)");
    ADD_BLOCK("any", "any(4,real)");

    ADD_BLOCK("unison_smf", "midireader(unison.midi)");
    ADD_BLOCK("melody", "midimelody");
    ADD_BLOCK("lpf_alpha", "constant(real,0.4)");
    ADD_BLOCK("lpf", "lpf");
    ADD_BLOCK("sound1", "wave(saw)");
    ADD_BLOCK("envelope", "envelope");

    ADD_BLOCK("drums_smf", "midireader(unison_drums.txt)");
    ADD_BLOCK("drums", "mididrums");
    ADD_BLOCK("kick_drum", "drum(kick)");
    ADD_BLOCK("snare_drum", "drum(snare)");
    ADD_BLOCK("white_drum", "drum(white)");

    ADD_BLOCK("vol_melody", "constant(real,0.10)");
    ADD_BLOCK("vol_kick", "constant(real,1.0)");
    ADD_BLOCK("vol_snare", "constant(real,0.3)");
    ADD_BLOCK("vol_white", "constant(real,0.1)");
    ADD_BLOCK("mix", "mixer(5)");
    ADD_BLOCK("compressor", "compressor(0.01)");
    ADD_BLOCK("soundcard", "wavfileout(new_unison.wav)");
    

    CONNECT("delta_time", "out", "time", "in");
    CONNECT("time", "out", "time_tee", "in");
    //CONNECT("time_tee", "main", "time_wye", "aux 1");
    CONNECT("time_tee", "aux 1", "ruler", "in");

    CONNECT("ruler", "out 16", "unison_smf", "in");
    CONNECT("unison_smf", "out", "melody", "in");
    CONNECT("melody", "pitch out", "lpf", "in");
    CONNECT("lpf_alpha", "out", "lpf", "alpha");
    CONNECT("lpf", "out", "sound1", "freq");
    CONNECT("melody", "velocity out", "envelope", "velocity in");
    CONNECT("sound1", "out", "envelope", "chunk in");
    CONNECT("envelope", "out", "mix", "in 1");
    CONNECT("vol_melody", "out", "mix", "gain 1");

    CONNECT("ruler", "out 4", "drums_smf", "in");
    CONNECT("drums_smf", "out", "drums", "midi");
    CONNECT("time_tee", "aux 2", "drums", "time");

    CONNECT("drums", "vel 0", "kick_drum", "velocity");
    CONNECT("drums", "time 0", "kick_drum", "time");
    CONNECT("kick_drum", "out", "mix", "in 2");
    CONNECT("vol_kick", "out", "mix", "gain 2");

    CONNECT("drums", "vel 1", "snare_drum", "velocity");
    CONNECT("drums", "time 1", "snare_drum", "time");
    CONNECT("snare_drum", "out", "mix", "in 3");
    CONNECT("vol_snare", "out", "mix", "gain 3");

    CONNECT("drums", "vel 2", "white_drum", "velocity");
    CONNECT("drums", "time 2", "white_drum", "time");
    CONNECT("white_drum", "out", "mix", "in 4");
    CONNECT("vol_white", "out", "mix", "gain 4");

    CONNECT("time_tee", "main", "mix", "gain 5"); // XXX hack until wye is fixed
    CONNECT("mix", "out", "compressor", "in");
    CONNECT("compressor", "out", "soundcard", "in");

    rc = nz_graph_block_handle(graph, "soundcard", &block_handle); if(rc != NZ_SUCCESS) goto err;
    //rc = pa_start(block_handle); if(rc != NZ_SUCCESS) goto err;
    int blocks_out = wavfileout_record(block_handle, 10);
    printf("Wrote %d blocks\n", blocks_out);

err:
    if (rc != NZ_SUCCESS) printf("Error on line: %s\n", err_line);
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

