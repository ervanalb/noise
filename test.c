#include <stdlib.h>
#include <stdio.h>
#include "debug.h"
#include "block.h"
#include "blockdef.h"

// TODO: Come up with a way better way of specifying constants
#define MAKE_CONSTANT(name, otype, ctype, value)   \
    object_t * name ## _obj = object_alloc(otype); \
    CAST_OBJECT(ctype, name ## _obj) = (value);    \
    node_t * name = constant_create(name ## _obj); 

#define MAKE_DOUBLE_CONSTANT(name, value) MAKE_CONSTANT(name, double_type, double, value)

int main(void) {
    //type_t * chunk_type = get_chunk_type();

    // Timebase
    MAKE_DOUBLE_CONSTANT(delta_t, 0.01);
    node_t * timebase = accumulator_create();
    node_t * time_tee = tee_create(2);
    node_t * time_wye = wye_create(2);
    node_connect(timebase, 0, delta_t, 0);
    node_connect(time_tee, 0, timebase, 0);
    node_connect(time_wye, 1, time_tee, 0);

    // Melody
    object_t * melody_obj = object_alloc(make_tuple_type(4));

    // TODO: Come up with a better way of specifying tuples
    CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[0] = object_alloc(double_type)) = 64;
    CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[1] = object_alloc(double_type)) = 66;
    //CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[2] = object_alloc(double_type)) = 63;
    CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[3] = object_alloc(double_type)) = 65;

    node_t * melody = constant_create(melody_obj);

    // Sequencer
    node_t * seq = sequencer_create();
    node_connect(seq, 0, time_tee, 1);
    node_connect(seq, 1, melody, 0);

    // Math
    node_t * n2f = math_create(MATH_NOTE_TO_FREQ);
    node_connect(n2f, 0, seq, 0);

    // Debug
    node_t * debug = debug_create();
    node_connect(debug, 0, time_wye, 0);

    // Instrument
    MAKE_CONSTANT(sine_wtype, long_type, long, WAVE_SINE);
    node_t * wave = wave_create();
    node_connect(wave, 0, n2f, 0);
    node_connect(wave, 1, sine_wtype, 0);


    // Mixer
    node_t * mixer = mixer_create(1);
    MAKE_DOUBLE_CONSTANT(wave_vol, 0.5);
    node_connect(mixer, 0, wave, 0);
    node_connect(mixer, 1, wave_vol, 0);
    node_connect(time_wye, 0, mixer, 0);


    // Soundcard 
    printf("Initing soundcard\n");
    node_t * soundcard = soundcard_get();
    node_connect(soundcard, 0, debug, 0);
    printf("soundcard inited\n");

    debug_print_graph(soundcard);
    soundcard_run();

    node_destroy(timebase);
    node_destroy(time_tee);
    node_destroy(melody);
    node_destroy(seq);
    node_destroy(wave);
    node_destroy(soundcard);

    printf("Successfully destroyed everything!\n");

    return 0;
}

/*
int main(void) {
    object_t * one_obj = object_alloc(double_type);
    CAST_OBJECT(double, one_obj) = 0.1;

    node_t * one = constant_create(one_obj);
    node_t * acc = accumulator_create();
    node_t * math = math_create(MATH_ADD);
    node_t * fungen = fungen_create();
    node_t * tee = tee_create(double_type, 2);
    node_t * wye = wye_create(double_type, 2);
    node_t * debug = debug_create(NULL);

    object_t * freq_obj = object_alloc(double_type);
    CAST_OBJECT(double, freq_obj) = 440.;
    node_t * freq = constant_create(freq_obj);

    object_t * type_sine_obj = object_alloc(long_type);
    CAST_OBJECT(long, one_obj) = WAVE_SINE;
    node_t * type_sine = constant_create(type_sine_obj);

    node_t * wave = wave_create();

    printf("Initing soundcard\n");
    node_t * soundcard = soundcard_get();
    node_connect(wave, 0, freq, 0);
    node_connect(wave, 1, type_sine, 0);
    node_connect(soundcard, 0, wave, 0);
    printf("soundcard inited\n");

    node_connect(acc, 0, one, 0);
    node_connect(fungen, 0, acc, 0);
    node_connect(debug, 0, fungen, 0);

    node_connect(math, 0, one, 0);
    node_connect(math, 1, one, 0);
    //node_connect(debug, 0, math, 0);

    node_connect(tee, 0, one, 0);
    node_connect(wye, 0, tee, 0);
    node_connect(wye, 1, tee, 1);
    //connect(debug, 0, wye, 0);

    debug_print_graph(debug);
    printf("\nRunning:\n");

    debug_run_n(debug, 10);
    soundcard_run();

    node_destroy(one);
    node_destroy(acc);
    node_destroy(math);
    node_destroy(fungen);
    node_destroy(tee);
    node_destroy(wye);
    node_destroy(debug);

    printf("Successfully destroyed everything!\n");

    return 0;
}
*/
