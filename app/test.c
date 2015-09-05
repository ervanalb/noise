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
            calloc(1, sizeof(struct nz_node *));    \
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

    struct nz_node * hits_node = calloc(1, sizeof(struct nz_node));
    struct nz_node * hits_seq = calloc(1, sizeof(struct nz_node));
    struct nz_node * voice = calloc(1, sizeof(struct nz_node));

    nz_constant_init(hits_node, hits_obj);
    nz_sequencer_init(hits_seq);
    nz_sampler_init(voice);

    nz_node_connect(hits_seq, 0, time_tee, tinp);
    nz_node_connect(hits_seq, 1, hits_node, 0);

    struct nz_node * debug = calloc(1, sizeof(struct nz_node));
    nz_debug_init(debug, "hit", 1);
    nz_node_connect(debug, 0, record, 0);
    nz_node_connect(voice, 0, debug, 0);
    nz_node_connect(voice, 1, hits_seq, 0);

    return voice;
}

int record_and_write(struct nz_node * recorder_node, const char * filename, double time_seconds) {
    DSL_DECLS;
    MAKE_LONG_CONSTANT(recorder_len, nz_frame_rate * time_seconds);
    nz_node_connect(recorder_node, 1, recorder_len, 0);

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
    MAKE_DOUBLE_CONSTANT(delta_t, nz_chunk_size / nz_frame_rate * 280 / 60.);
    struct nz_node timebase[1];
    struct nz_node time_tee[1];
    struct nz_node time_wye[1];

    nz_accumulator_init(timebase);
    nz_tee_init(time_tee, 4);
    nz_wye_init(time_wye, 2);

    nz_node_connect(timebase, 0, delta_t, 0);
    nz_node_connect(time_tee, 0, timebase, 0);

    // Debug
    struct nz_node debug_time[1];
    nz_debug_init(debug_time, "time", 0);
    nz_node_connect(debug_time, 0, time_tee, 0);
    nz_node_connect(time_wye, 1, debug_time, 0);

    // Melody
    // TODO: Come up with a better way of specifying tuples
    double unison[] = {None, None, None, 65, 75, None, 72, 67, 67, 68, None, 65, 70, 72, 70, 65, 65, None, None, 65, 75, None, 72, 67, 67, 68, 65, 72, 75, None, 72, 77};
    size_t unison_len = sizeof(unison) / sizeof(*unison);

    struct nz_obj * melody_obj = make_double_vector(unison,  unison_len);
    struct nz_node melody[1];
    nz_constant_init(melody, melody_obj);

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

    // Snare
    struct nz_node snare_imp[1];
    struct nz_node snare_lpf[1];
    struct nz_node snare_rec[1];
    struct nz_node snare_wav[1];
    struct nz_node snare_mix[1];

    nz_impulse_init(snare_imp);
    MAKE_DOUBLE_CONSTANT(snare_tau, 8.5);
    nz_clpf_init(snare_lpf);
    nz_node_connect(snare_lpf, 0, snare_imp, 0);
    nz_node_connect(snare_lpf, 1, snare_tau, 0);

    nz_white_init(snare_wav);
    nz_cmixer_init(snare_mix, 1);
    nz_node_connect(snare_mix, 0, snare_wav, 0);
    nz_node_connect(snare_mix, 1, snare_lpf, 0);

    nz_recorder_init(snare_rec);
    MAKE_LONG_CONSTANT(snare_len, nz_frame_rate * 0.5);
    nz_node_connect(snare_rec, 0, snare_mix, 0);
    nz_node_connect(snare_rec, 1, snare_len, 0);
    //nz_record_and_write(snare_rec, "snare.wav", 0.5);

    // Kick
    struct nz_node kick_imp[1];
    struct nz_node kick_lpf[1];
    struct nz_node kick_rec[1];
    struct nz_node kick_wav[1];
    struct nz_node kick_mix[1];
    nz_impulse_init(kick_imp);
    MAKE_DOUBLE_CONSTANT(kick_tau, 8.5);
    nz_clpf_init(kick_lpf);
    nz_node_connect(kick_lpf, 0, kick_imp, 0);
    nz_node_connect(kick_lpf, 1, kick_tau, 0);

    MAKE_DOUBLE_CONSTANT(kick_freq, 80);
    MAKE_LONG_CONSTANT(kick_wtype, NZ_WAVE_SINE);
    nz_wave_init(kick_wav);
    nz_node_connect(kick_wav, 0, kick_freq, 0);
    nz_node_connect(kick_wav, 1, kick_wtype, 0);

    nz_cmixer_init(kick_mix, 1);
    nz_node_connect(kick_mix, 0, kick_wav, 0);
    nz_node_connect(kick_mix, 1, kick_lpf, 0);

    nz_recorder_init(kick_rec);
    MAKE_LONG_CONSTANT(kick_len, nz_frame_rate * 0.5);
    nz_node_connect(kick_rec, 0, kick_mix, 0);
    nz_node_connect(kick_rec, 1, kick_len, 0);
    

    // Kick & Snare sampling
    long snare_pat[] = {NZ_SAMPLER_COMMAND_PLAY, NZ_SAMPLER_COMMAND_PLAY, NZ_SAMPLER_COMMAND_STOP, NZ_SAMPLER_COMMAND_STOP};
    long kick_pat[]  = {NZ_SAMPLER_COMMAND_STOP, NZ_SAMPLER_COMMAND_STOP, NZ_SAMPLER_COMMAND_PLAY, NZ_SAMPLER_COMMAND_PLAY};

    struct nz_node * snare = make_drum(snare_rec, snare_pat, 4, time_tee, 2);
    struct nz_node * kick = make_drum(kick_rec, kick_pat, 4, time_tee, 3);


    /*
    long snare_hits[] = {0,0,1,1};
    double snare_samples[50000];
    double snare_tau = 0.0001;
    
    for (size_t i = 0; i < 50000; i++)
        snare_samples[i] = ((rand() / (double) (RAND_MAX / 2)) - 1.0) * exp(- (double) i * snare_tau);

    struct nz_node * snare = make_drum(snare_samples, 50000, snare_hits, 4, time_tee, 2);

    struct nz_node * debug_snare = debug_create("snare", 1);
    node_connect(debug_snare, 0, snare, 0);

    long kick_hits[] = {0,0,1,1};
    double kick_samples[50000];
    double kick_tau = 0.0001;
    double kick_freq = 60;

    for (size_t i = 0; i < 50000; i++)
        kick_samples[i] = cos(2 * M_PI * i * kick_freq / global_frame_rate) * exp(- (double) i * kick_tau);

    struct nz_node * kick = make_drum(kick_samples, 50000, kick_hits, 4, time_tee, 3);
    MAKE_DOUBLE_CONSTANT(kick_vol, 0.4);
    */

    // Mixer
    struct nz_node mixer[1];
    nz_mixer_init(mixer, 3);
    MAKE_DOUBLE_CONSTANT(wave_vol, 0.40);
    MAKE_DOUBLE_CONSTANT(snare_vol, 0.80);
    MAKE_DOUBLE_CONSTANT(kick_vol, 2.0);

    nz_node_connect(mixer, 0, wave, 0);
    nz_node_connect(mixer, 1, wave_vol, 0);
    nz_node_connect(mixer, 2, snare, 0);
    nz_node_connect(mixer, 3, snare_vol, 0);
    nz_node_connect(mixer, 4, kick, 0);
    nz_node_connect(mixer, 5, kick_vol, 0);

    nz_node_connect(time_wye, 0, mixer, 0);
    
    // Debug
    struct nz_node debug_ch[1];
    nz_debug_init(debug_ch, "ch", 0);
    nz_node_connect(debug_ch, 0, time_wye, 0);

    // Compressor
    struct nz_node compressor[1];
    nz_compressor_init(compressor);
    nz_node_connect(compressor, 0, debug_ch, 0);

    struct nz_node recorder[1];
    nz_recorder_init(recorder);
    //MAKE_LONG_CONSTANT(recorder_len, nz_chunk_size * 5);
    nz_node_connect(recorder, 0, compressor, 0);

    nz_debug_print_graph(recorder);

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
