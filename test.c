#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sndfile.h>
#include "block.h"
#include "blockdef.h"
#include "debug.h"
#include "noise.h"
#include "util.h"
#include "noise.h"

const size_t noise_chunk_size = 128;
const double noise_frame_rate = 44100;


// TODO: Come up with a way better way of specifying constants
#define MAKE_CONSTANT(name, otype, ctype, value)    \
    object_t * name ## _obj = object_alloc(otype);  \
    CAST_OBJECT(ctype, name ## _obj) = (value);     \
    node_t name;                                    \
    constant_init(&name, name ## _obj); 

#define MAKE_DOUBLE_CONSTANT(name, value) MAKE_CONSTANT(name, double_type, double, value)
#define MAKE_LONG_CONSTANT(name, value) MAKE_CONSTANT(name, long_type, long, value)


/*
node_t * make_drum(const double * sample, size_t sample_len, const long * hits, size_t hits_len, node_t * time_tee, size_t tinp)
{
    object_t * hits_obj = object_alloc(make_tuple_type(hits_len));
    for (size_t i = 0; i < hits_len; i++) {
        object_t * h = object_alloc(long_type);
        CAST_OBJECT(long, h) = hits[i];
        (&CAST_OBJECT(object_t *, hits_obj))[i] = h;
    }
    node_t * hits_node = constant_create(hits_obj);
    node_t * hits_seq = sequencer_create();
    node_connect(hits_seq, 0, time_tee, tinp);
    node_connect(hits_seq, 1, hits_node, 0);
    node_t * voice = sampler_create(sample, sample_len);
    node_connect(voice, 0, hits_seq, 0);

    return voice;
}
*/

int main(void) {
    //type_t * chunk_type = get_chunk_type();

    // Timebase
    MAKE_DOUBLE_CONSTANT(delta_t, 0.01);
    node_t timebase, time_tee, time_wye;
    accumulator_init(&timebase);
    tee_init(&time_tee, 4);
    wye_init(&time_wye, 2);

    node_connect(&timebase, 0, &delta_t, 0);
    node_connect(&time_tee, 0, &timebase, 0);

    // Debug
    node_t debug_time;
    debug_init(&debug_time, "time", 0);
    node_connect(&debug_time, 0, &time_tee, 0);
    node_connect(&time_wye, 1, &debug_time, 0);

    // Melody
    // TODO: Come up with a better way of specifying tuples
#define None -1
    double unison[] = {88, None, None, 65, 75, None, 72, 67, 67, 68, None, 65, 70, 72, 70, 65, 65, None, None, 65, 75, None, 72, 67, 67, 68, 65, 72, 75, None, 72, 77};
    size_t unison_len = sizeof(unison) / sizeof(*unison);

    object_t * melody_obj = object_alloc(get_object_vector_type());
    vector_set_size(melody_obj, unison_len);

    for (size_t i = 0; i < unison_len; i++) {
        if (unison[i] == None) continue;

        object_t * note = object_alloc(double_type);
        CAST_OBJECT(double, note) = unison[i];
        CAST_OBJECT(object_t **, melody_obj)[i] = note;
    }

    node_t melody;
    constant_init(&melody, melody_obj);

    // Sequencer
    node_t seq;
    sequencer_init(&seq);
    node_connect(&seq, 0, &time_tee, 1);
    node_connect(&seq, 1, &melody, 0);

    // Debug
    node_t debug_seq;
    debug_init(&debug_seq, "seq", 0);
    node_connect(&debug_seq, 0, &seq, 0);

    node_t lpf;
    lpf_init(&lpf);
    MAKE_CONSTANT(lpf_alpha, double_type, double, 2);
    node_connect(&lpf, 0, &debug_seq, 0);
    node_connect(&lpf, 1, &lpf_alpha, 0);

    // Debug
    node_t debug_lpf;
    debug_init(&debug_lpf, "lpf", 0);
    node_connect(&debug_lpf, 0, &lpf, 0);

    // Math
    node_t n2f; 
    math_init(&n2f, MATH_NOTE_TO_FREQ);
    node_connect(&n2f, 0, &debug_lpf, 0);

    // Instrument
    MAKE_CONSTANT(wtype, long_type, long, WAVE_SAW);
    node_t wave;
    wave_init(&wave);
    node_connect(&wave, 0, &n2f, 0);
    node_connect(&wave, 1, &wtype, 0);
    /*

    long snare_hits[] = {0,0,1,1};
    double snare_samples[50000];
    double snare_tau = 0.0001;
    
    for (size_t i = 0; i < 50000; i++)
        snare_samples[i] = ((rand() / (double) (RAND_MAX / 2)) - 1.0) * exp(- (double) i * snare_tau);

    node_t * snare = make_drum(snare_samples, 50000, snare_hits, 4, time_tee, 2);
    MAKE_DOUBLE_CONSTANT(snare_vol, 0.4);

    node_t * debug_snare = debug_create("snare", 1);
    node_connect(debug_snare, 0, snare, 0);

    long kick_hits[] = {0,0,1,1};
    double kick_samples[50000];
    double kick_tau = 0.0001;
    double kick_freq = 60;

    for (size_t i = 0; i < 50000; i++)
        kick_samples[i] = cos(2 * M_PI * i * kick_freq / global_frame_rate) * exp(- (double) i * kick_tau);

    node_t * kick = make_drum(kick_samples, 50000, kick_hits, 4, time_tee, 3);
    MAKE_DOUBLE_CONSTANT(kick_vol, 0.4);
    */

    // Mixer
    node_t  mixer;
    mixer_init(&mixer, 3);
    MAKE_DOUBLE_CONSTANT(wave_vol, 0.05);

    node_connect(&mixer, 0, &wave, 0);
    node_connect(&mixer, 1, &wave_vol, 0);
    /*
    node_connect(mixer, 2, debug_snare, 0);
    node_connect(mixer, 3, snare_vol, 0);
    node_connect(mixer, 4, kick, 0);
    node_connect(mixer, 5, kick_vol, 0);
    */

    node_connect(&time_wye, 0, &mixer, 0);
    
    // Debug
    node_t debug_ch;
    debug_init(&debug_ch, "ch", 0);
    node_connect(&debug_ch, 0, &time_wye, 0);

    node_t recorder;
    recorder_init(&recorder);
    MAKE_LONG_CONSTANT(recorder_len, noise_frame_rate * 30);
    //MAKE_LONG_CONSTANT(recorder_len, noise_chunk_size * 5);
    node_connect(&recorder, 0, &debug_ch, 0);
    node_connect(&recorder, 1, &recorder_len, 0);

    debug_print_graph(&recorder);

    // This triggers everything!
    object_t * sample = port_pull(&recorder.node_outputs[0]);
    printf("%s %lu", object_str(sample), vector_get_size(sample));

    SF_INFO fdata = {
        .frames = vector_get_size(sample),
        .samplerate = noise_frame_rate,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16,
        .sections = 1,
        .seekable = 0,
    };
    SNDFILE * f = sf_open("unison.wav", SFM_WRITE, &fdata);
    sf_write_double(f, CAST_OBJECT(double *, sample), vector_get_size(sample));
    sf_write_sync(f);
    sf_close(f);


    /*
    // Soundcard 
    printf("Initing soundcard\n");
    node_t * soundcard = soundcard_get();
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
