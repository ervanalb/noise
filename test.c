#include <stdlib.h>
#include <stdio.h>
#include "debug.h"
#include "block.h"
#include "blockdef.h"

int main(void) {
    object_t * one_obj = object_alloc(double_type);
    CAST_OBJECT(double, one_obj) = 0.1;

    node_t * one = constant_create(one_obj);
    node_t * acc = accumulator_create(NULL);
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
