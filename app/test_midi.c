#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sndfile.h>

#include "noise.h"
#include "debug.h"
#include "blocks/blocks.h"
#include "blocks/io/midi.h"
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

struct nz_node * make_drum(struct nz_node * record, const long * hits, size_t hits_len, struct nz_node * time_tee, size_t tinp) {
    struct nz_obj * hits_obj = nz_obj_create(nz_object_vector_type);
    DSL_DECLS;
    nz_vector_set_size(hits_obj, hits_len);

    for (size_t i = 0; i < hits_len; i++) {
        struct nz_obj * v = nz_obj_create(nz_long_type);
        NZ_CAST(long, v) = hits[i];
        NZ_CAST(struct nz_obj **, hits_obj)[i] = v;
    }

    MBLOCK(hits_node, constant, hits_obj);
    MBLOCK(hits_seq, sequencer);

    CONNECT(hits_seq, 0, time_tee, tinp);
    CONNECT(hits_seq, 1, hits_node, 0);

    MBLOCK(debug, debug, "hit", 1);
    CONNECT(debug, 0, record, 0);

    MBLOCK(voice, sampler);
    CONNECT(voice, 0, debug, 0);
    CONNECT(voice, 1, hits_seq, 0);

    return voice;
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

    BLOCK(time_tee, tee, 5);
    CONNECT(time_tee, 0, _pipe, 0);

    // Debug
    BLOCK(debug_time, debug, "time", 0);
    CONNECT(_blk, 0, _pipe, 0);

    BLOCK(time_wye, wye, 3);
    CONNECT(time_wye, 1, _pipe, 0);

    // MIDI
    BLOCK(midi_events, midireader, "example.smf");
    CONNECT(midi_events, 0, time_tee, 4);
    BLOCK(midi_notes, midiintegrator);
    CONNECT(midi_notes, 0, midi_events, 0);
    
    BLOCK(instrument, instrument, nz_sine_state_size, nz_sine_render);
    CONNECT(_blk, 0, _pipe, 0);

    //CONNECT(time_wye, 2, midi_notes, 0);
/*
    // Melody
    // TODO: Come up with a better way of specifying these
    double unison[] = {None, None, None, 65, 75, None, 72, 67, 67, 68, None, 65, 70, 72, 70, 65, 65, None, None, 65, 75, None, 72, 67, 67, 68, 65, 72, 75, None, 72, 77};
    size_t unison_len = sizeof(unison) / sizeof(*unison);

    struct nz_obj * melody_obj = make_double_vector(unison,  unison_len);
    BLOCK(melody, constant, melody_obj);

    // Sequencer
    BLOCK(seq, sequencer);
    CONNECT(_blk, 0, time_tee, 1);
    CONNECT(_blk, 1, melody, 0);

    // Debug
    BLOCK(debug_seq, debug, "seq", 0);
    CONNECT(_blk, 0, seq, 0);

    BLOCK(lpf, lpf);
    CONNECT(_blk, 0, _pipe, 0);
    MAKE_CONSTANT(lpf_alpha, nz_double_type, double, 2);
    CONNECT(_blk, 1, _cons, 0);

    // Debug
    BLOCK(debug_lpf, debug, "lpf", 0);
    CONNECT(_blk, 0, _pipe, 0);

    // Math
    BLOCK(n2f, math, NZ_MATH_NOTE_TO_FREQ);
    CONNECT(_blk, 0, _pipe, 0);

    // Instrument
    BLOCK(wave, wave);
    CONNECT(_blk, 0, _pipe, 0);
    MAKE_CONSTANT(wtype, nz_long_type, long, NZ_WAVE_SAW);
    CONNECT(_blk, 1, _cons, 0);

    */

    // --- Percussion ---

    // -- Snare --
    BLOCK(snare_imp, impulse);

    BLOCK(snare_lpf, clpf);
    CONNECT(_blk, 0, snare_imp, 0);
    MAKE_DOUBLE_CONSTANT(snare_tau, 8.5);
    CONNECT(_blk, 1, _cons, 0);

    BLOCK(snare_wav, white);

    BLOCK(snare_mix, cmixer, 1);
    CONNECT(_blk, 0, snare_wav, 0);
    CONNECT(_blk, 1, snare_lpf, 0);

    BLOCK(snare_rec, recorder);
    CONNECT(_blk, 0, _pipe, 0);
    MAKE_LONG_CONSTANT(snare_len, nz_frame_rate * 0.5);
    CONNECT(_blk, 1, _cons, 0);
    //nz_record_and_write(snare_rec, "snare.wav", 0.5);

    // -- Kick --
    BLOCK(kick_imp, impulse);

    MAKE_DOUBLE_CONSTANT(kick_tau, 8.5);
    BLOCK(kick_lpf, clpf);
    CONNECT(_blk, 0, kick_imp, 0);
    CONNECT(_blk, 1, kick_tau, 0);

    BLOCK(kick_wav, wave);
    MAKE_DOUBLE_CONSTANT(kick_freq, 80);
    CONNECT(_blk, 0, _cons, 0);
    MAKE_LONG_CONSTANT(kick_wtype, NZ_WAVE_SINE);
    CONNECT(_blk, 1, _cons, 0);

    BLOCK(kick_mix, cmixer, 1);
    CONNECT(_blk, 0, kick_wav, 0);
    CONNECT(_blk, 1, kick_lpf, 0);

    BLOCK(kick_rec, recorder);
    CONNECT(_blk, 0, _pipe, 0);
    MAKE_LONG_CONSTANT(kick_len, nz_frame_rate * 0.5);
    CONNECT(_blk, 1, _cons, 0);

    // Kick & Snare sampling
    long snare_pat[] = {NZ_SAMPLER_COMMAND_PLAY, NZ_SAMPLER_COMMAND_PLAY, NZ_SAMPLER_COMMAND_STOP, NZ_SAMPLER_COMMAND_STOP};
    long kick_pat[]  = {NZ_SAMPLER_COMMAND_STOP, NZ_SAMPLER_COMMAND_STOP, NZ_SAMPLER_COMMAND_PLAY, NZ_SAMPLER_COMMAND_PLAY};

    struct nz_node * snare = make_drum(snare_rec, snare_pat, 4, time_tee, 2);
    struct nz_node * kick = make_drum(kick_rec, kick_pat, 4, time_tee, 3);

    // Mixer
    BLOCK(mixer, mixer, 3);

    MAKE_DOUBLE_CONSTANT(wave_vol, 0.40);
    MAKE_DOUBLE_CONSTANT(snare_vol, 0.00);
    MAKE_DOUBLE_CONSTANT(kick_vol, 0.0);

    CONNECT(mixer, 0, instrument, 0);
    CONNECT(mixer, 1, wave_vol, 0);
    CONNECT(mixer, 2, snare, 0);
    CONNECT(mixer, 3, snare_vol, 0);
    CONNECT(mixer, 4, kick, 0);
    CONNECT(mixer, 5, kick_vol, 0);

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
    nz_debug_print_dot(recorder, "unison.dot");

    // This triggers everything!
    record_and_write(recorder, "unison.wav", 10);


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
