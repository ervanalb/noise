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
    /*
    object_t * melody_obj = object_alloc(make_tuple_type(4));

    // TODO: Come up with a better way of specifying tuples
    CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[0] = object_alloc(double_type)) = 64;
    CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[1] = object_alloc(double_type)) = 66;
    //CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[2] = object_alloc(double_type)) = 63;
    CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[3] = object_alloc(double_type)) = 65;
    */

    //XXX 
#define None -1
    double unison[] = {None, None, None, 65, 75, None, 72, 67, 67, 68, None, 65, 70, 72, 70, 65, 65, None, None, 65, 75, None, 72, 67, 67, 68, 65, 72, 75, None, 72, 77};
    size_t unison_len = sizeof(unison) / sizeof(*unison);

    object_t * melody_obj = object_alloc(make_tuple_type(unison_len));

    for (size_t i = 0; i < unison_len; i++) {
        if (unison[i] != None)
            CAST_OBJECT(double, (&CAST_OBJECT(object_t *, melody_obj))[i] = object_alloc(double_type)) = unison[i];
    }

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
    MAKE_CONSTANT(wtype, long_type, long, WAVE_SAW);
    node_t * wave = wave_create();
    node_connect(wave, 0, n2f, 0);
    node_connect(wave, 1, wtype, 0);

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

    node_destroy(debug);
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

    return 0;
}
