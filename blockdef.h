#ifndef __BLOCKDEF_H__
#define __BLOCKDEF_H__

#include "block.h"

enum math_op {
    MATH_ADD,
    MATH_SUBTRACT,
    MATH_MULTIPLY,
    MATH_DIVIDE,
    MATH_NOTE_TO_FREQ
};

enum wave_type {
    WAVE_SINE,
    WAVE_SAW,
    WAVE_SQUARE,
};

// Block defs

// Accumulator<> :: (double x) -> (double x_sum); Sums an input
node_t * accumulator_create();

// Constant<type value> :: () -> (type value); Returns a constant value
node_t * constant_create(object_t * constant_value);

// Debug<char name[]> :: (double x) -> (); Prints output on pull; or use with debug_run(...)
node_t * debug_create(const char *);

// FunGen<> :: (double t) -> (double x); Sine fn generator. Computes x = sin(t)
node_t * fungen_create();

// LPF<> :: (double x, double alpha) -> (double x_lpf); Low pass `x` with time const `alpha` (in pulls)
node_t * lpf_create();

// Math<math_op> :: (double x[, double y]) -> (double result); Performs a basic math op on inputs (see enum math_op)
node_t * math_create(enum math_op op);

// Mixer<n_channels> :: (chunk s, double vol[, chunk s2, double vol2, ...]) -> (chunk mixout); 
node_t * mixer_create(size_t n_channels);

// Sequencer<> :: (double time, tuple<> stream) -> (tuple_els elem);
node_t * sequencer_create();

// Tee<int n_inputs> :: (type val) -> (type val[, type copy, type copy, ...]); Pulls from input, duplicates
node_t * tee_create(size_t n_inputs);

// Wye<int n_inputs> :: (type val[, type discard, type discard, ...]) -> (type val); Pulls from all inputs, returning the first
node_t * wye_create(size_t n_inputs);

// Wave :: (double frequency, long wave_type) -> (chunk samples); See enum wave_type
node_t * wave_create();

// Soundcard Sink :: (chunk stream) -> ();
node_t * soundcard_get(); // Singleton
void soundcard_run();

#endif
