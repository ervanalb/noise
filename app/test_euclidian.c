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

void generate_euclidean_delta(int k, int n, int * out) { // out has size k
    assert(n > 0);
    assert(k > 0);
    assert(n >= k);
    int a = n % k;

    if (a == 0) {
        for (int i = 0; i < k; i++) {
            out[i] = n / k;
        }
    } else {
        int rec[a];
        generate_euclidean_delta(a, k, rec);
        int flr = n / k;
        int cel = flr + 1;
        for (int i = a - 1; i >= 0; i--) {
            *out++ = cel;
            for (int j = 0; j < rec[i] - 1; j++) {
                *out++ = flr;
            }
        }
    }
}

void flatten_delta(int k, int n, int * in, int * out) { // in has size k, out has size n
    memset(out, 0, n * sizeof(*out));
    while (k) {
        *out = 1;
        out += *in; k--;
        n -= *in++;
    }
    assert(n == 0);
}

int main(void) {
    int kd = 7, ks=5, n = 16;
    double frac = 0.5;

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

    // Time modulo for looping
    BLOCK(time_mod_, math, NZ_MATH_MODULO);
    CONNECT(_blk, 0, time_tee, 1);
    MAKE_DOUBLE_CONSTANT(bar_len, n * frac);
    CONNECT(_blk, 1, _cons, 0);

    BLOCK(time_mod, debug, "time_mod", 0);
    CONNECT(time_mod, 0, time_mod_, 0);


    // Snare rhythm
    int rhy_delta[n];
    int rhy[n];

    generate_euclidean_delta(kd, n, rhy_delta);
    flatten_delta(kd, n, rhy_delta, rhy);
    printf("Euclidean rhythm snare (%d, %d): ", kd, n);
    for (int i = 0; i < n; i++) {
        printf("%c", rhy[i] ? 'x' : '.');
    }
    printf("\n");
    
    double rhy_sum = 0;
    struct nz_obj * rhythm_tnotes = nz_obj_create(nz_tnote_vector_type);
    nz_vector_set_size(rhythm_tnotes, kd);
    for (int i = 0; i < kd; i++) {
        nz_tnote_init(nz_vector_at(rhythm_tnotes, i), 1, 1, rhy_sum, 0.1);
        rhy_sum += frac * rhy_delta[i];
    }
    BLOCK(snare_notes, constant, rhythm_tnotes);
    BLOCK(snare_seq, notesequencer);
    CONNECT(_blk, 0, time_mod, 0);
    CONNECT(_blk, 1, snare_notes, 0);

    BLOCK(snare, instrument_snare);
    CONNECT(_blk, 0, _pipe, 0);

    // Synth rhythm
    generate_euclidean_delta(ks, n, rhy_delta);
    flatten_delta(ks, n, rhy_delta, rhy);
    printf("Euclidean rhythm synth (%d, %d): ", ks, n);
    for (int i = 0; i < n; i++) {
        printf("%c", rhy[i] ? 'x' : '.');
    }
    printf("\n");
    
    rhy_sum = 0;
    struct nz_obj * synth_tnotes = nz_obj_create(nz_tnote_vector_type);
    nz_vector_set_size(synth_tnotes, ks);
    double pitch = 65;
    int delta_pitches[8] = {0, 3, 4, 5, -7, -4, -5, 7};
    for (int i = 0; i < ks; i++) {
        nz_tnote_init(nz_vector_at(synth_tnotes, i), nz_note_to_freq(pitch), 1, rhy_sum, 2.5 * frac);
        rhy_sum += frac * rhy_delta[i];
        pitch += delta_pitches[(int) (rand() / (double) (RAND_MAX / 8))];
    }
    BLOCK(synth_notes, constant, synth_tnotes);
    BLOCK(synth_seq, notesequencer);
    CONNECT(_blk, 0, time_mod, 0);
    CONNECT(_blk, 1, synth_notes, 0);

    BLOCK(synth, instrument_saw);
    CONNECT(_blk, 0, _pipe, 0);

    // Mixer
    BLOCK(mixer, mixer, 3);

    MAKE_DOUBLE_CONSTANT(wave_vol, 0.40);
    MAKE_DOUBLE_CONSTANT(snare_vol, 0.60);
    MAKE_DOUBLE_CONSTANT(kick_vol, 2.0);

    CONNECT(mixer, 0, synth, 0);
    CONNECT(mixer, 1, wave_vol, 0);
    CONNECT(mixer, 2, snare, 0);
    CONNECT(mixer, 3, snare_vol, 0);
    //CONNECT(mixer, 4, kick, 0);
    //CONNECT(mixer, 5, kick_vol, 0);

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
    nz_debug_print_dot(recorder, "euclidian.dot");

    // This triggers everything!
    record_and_write(recorder, "euclidian.wav", 10);

    return 0;
}
