#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sndfile.h>

#include "noise.h"
#include "debug.h"
#include "blocks/blocks.h"
#include "core/util.h"

const size_t nz_chunk_size = 128;
const double nz_frame_rate = 44100;

// TODO: Come up with a way better way of specifying constants
#define MAKE_CONSTANT(name, otype, ctype, value)            \
    struct nz_obj * name ## _obj = nz_obj_create(otype);    \
    NZ_CAST(ctype, name ## _obj) = (value);                 \
    struct nz_node name[1];                                 \
    nz_constant_init(name, name ## _obj);                   \
    _cons = name;

#define MAKE_DOUBLE_CONSTANT(name, value) MAKE_CONSTANT(name, nz_double_type, double, value)
#define MAKE_LONG_CONSTANT(name, value) MAKE_CONSTANT(name, nz_long_type, long, value)

#define BLOCK(varname, name, ...)                   \
    struct nz_node varname[1];                      \
    nz_ ## name ## _init(varname, ## __VA_ARGS__);  \
    _blk = varname;

#define MBLOCK(varname, name, ...)                  \
    struct nz_node * varname =                      \
            calloc(1, sizeof(struct nz_node));    \
    assert(varname != NULL);                        \
    nz_ ## name ## _init(varname, ## __VA_ARGS__);  \
    _blk = varname;

#define CONNECT(out, ox, in, ix)        \
    nz_node_connect(out, ox, in, ix);   \
    _pipe = out;

#define DSL_DECLS           \
    struct nz_node * _pipe; \
    struct nz_node * _cons; \
    struct nz_node * _blk;  

#define None -1
struct nz_obj * make_double_vector(double * array, size_t len) {
    DSL_DECLS;
    struct nz_obj * obj = nz_obj_create(nz_object_vector_type);
    nz_vector_set_size(obj, len);

    for (size_t i = 0; i < len; i++) {
        if (array[i] == None) continue;

        struct nz_obj * v = nz_obj_create(nz_double_type);
        NZ_CAST(double, v) = array[i];
        NZ_CAST(struct nz_obj **, obj)[i] = v;
    }
    return obj;
}

int record_and_write(struct nz_node * recorder_node, const char * filename, double time_seconds) {
    DSL_DECLS;
    MAKE_LONG_CONSTANT(recorder_len, nz_frame_rate * time_seconds);
    CONNECT(recorder_node, 1, recorder_len, 0);

    // Trigger the computation
    struct nz_obj * sample = nz_port_pull(&recorder_node->node_outputs[0]);
    printf("Writing %lu frames to %s", nz_vector_get_size(sample), filename);

    SF_INFO fdata = {
        .frames = nz_vector_get_size(sample),
        .samplerate = nz_frame_rate,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16,
        .sections = 1,
        .seekable = 0,
    };

    SNDFILE * f = sf_open(filename, SFM_WRITE, &fdata);
    sf_write_double(f, NZ_CAST(double *, sample), nz_vector_get_size(sample));
    sf_write_sync(f);
    sf_close(f);
    return 0;
}

int main(void) {
    DSL_DECLS;
    // Timebase
    // (frames / chunk) / (frames / second) * (beats / minute) / (seconds / minute) 
    // = (frames * second * beats * minute) / (chunk * frames * minute * second) 
    // = (beats / chunk)

    BLOCK(timebase, accumulator);
    MAKE_DOUBLE_CONSTANT(delta_t, nz_chunk_size / nz_frame_rate * 280 / 60.);
    CONNECT(_blk, 0, _cons, 0);

    BLOCK(time_tee, tee, 4);
    CONNECT(time_tee, 0, _pipe, 0);

    // Debug
    BLOCK(debug_time, debug, "time", 0);
    CONNECT(_blk, 0, _pipe, 0);

    BLOCK(time_wye, wye, 2);
    CONNECT(time_wye, 1, _pipe, 0);

    // Melody
    struct nz_note notes[4];
    nz_note_init(&notes[0], 65, 1);
    nz_note_init(&notes[1], 67, 1);
    nz_note_init(&notes[2], 69, 1);
    nz_note_init(&notes[3], 72, 1);

    struct nz_obj * nvecs = nz_obj_create(nz_object_vector_type);
    // 0
    struct nz_obj * nv = nz_obj_create(nz_note_vector_type);
    nz_vector_push_back(nvecs, &nv);
    nz_vector_push_back(nv, &notes[0]);

    // 1
    nv = nz_obj_create(nz_note_vector_type);
    nz_vector_push_back(nvecs, &nv);

    // 2
    nv = nz_obj_create(nz_note_vector_type);
    nz_vector_push_back(nvecs, &nv);
    nz_vector_push_back(nv, &notes[1]);

    // 3
    nv = nz_obj_create(nz_note_vector_type);
    nz_vector_push_back(nvecs, &nv);
    nz_vector_push_back(nv, &notes[1]);
    nz_vector_push_back(nv, &notes[3]);

    // 4
    nv = nz_obj_create(nz_note_vector_type);
    nz_vector_push_back(nvecs, &nv);
    nz_vector_push_back(nv, &notes[3]);

    // 5
    nv = nz_obj_create(nz_note_vector_type);
    nz_vector_push_back(nvecs, &nv);

    // 6
    nv = nz_obj_create(nz_note_vector_type);
    nz_vector_push_back(nvecs, &nv);
    nz_vector_push_back(nv, &notes[2]);

    // 7
    nv = nz_obj_create(nz_note_vector_type);
    nz_vector_push_back(nvecs, &nv);
    nz_vector_push_back(nv, &notes[2]);

    BLOCK(melody, constant, nvecs);

    // Sequencer
    BLOCK(seq, sequencer);
    CONNECT(_blk, 0, time_tee, 1);
    CONNECT(_blk, 1, melody, 0);

    // Debug
    BLOCK(debug_seq, debug, "seq", 0);
    CONNECT(_blk, 0, seq, 0);

    // Instrument
    BLOCK(synth, synth);
    CONNECT(_blk, 0, _pipe, 0);


    // Mixer
    BLOCK(mixer, mixer, 1);

    MAKE_DOUBLE_CONSTANT(synth_vol, 0.25);

    CONNECT(mixer, 0, synth, 0);
    CONNECT(mixer, 1, synth_vol, 0);

    CONNECT(time_wye, 0, mixer, 0);
    
    // Debug
    BLOCK(debug_ch, debug, "ch", 0);
    CONNECT(_blk, 0, _pipe, 0);

    // Compressor
    BLOCK(compressor, compressor);
    CONNECT(_blk, 0, _pipe, 0);

    // Recorder
    BLOCK(recorder, recorder);
    CONNECT(_blk, 0, _pipe, 0);

    nz_debug_print_graph(recorder);
    nz_debug_print_dot(recorder, "synth.dot");

    // This triggers everything!
    record_and_write(recorder, "synth.wav", 10);


    /*
    // Soundcard 
    printf("Initing soundcard\n");
    struct nz_node * soundcard = soundcard_get();
    node_connect(soundcard, 0, time_wye, 0);
    printf("soundcard inited\n");

    debug_print_graph(soundcard);
    soundcard_run();

    node_destroy(delta_t);
    node_destroy(melody);
    node_destroy(mixer);
    node_destroy(n2f);
    node_destroy(seq);
    node_destroy(soundcard);
    node_destroy(time_tee);
    node_destroy(time_wye);
    node_destroy(timebase);
    node_destroy(wave);
    node_destroy(wave_vol);
    node_destroy(wtype);

    printf("Successfully destroyed everything!\n");
    */

    return 0;
}
