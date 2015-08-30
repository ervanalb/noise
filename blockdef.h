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
    WAVE_WHITE,
};

enum sampler_command {
    // Pause: output silent chunks, keep current seek position
    SAMPLER_COMMAND_PAUSE,
    // Play: output chunks from sample until the end is reached
    SAMPLER_COMMAND_PLAY,
    // Restart: seek back to beginning of sample & start playing
    SAMPLER_COMMAND_RESTART,
    // Stop: seek back to beginning of sample & pause
    SAMPLER_COMMAND_STOP,
    // Refetch: pull in a new sample from the sample port
    SAMPLER_COMMAND_REFETCH,
};

// Block defs

// Accumulator<> :: (double x) -> (double x_sum); Sums an input
int accumulator_init(node_t * node);

// Constant<type value> :: () -> (type value); Returns a constant value
int constant_init(node_t * node, object_t * constant_value);

// Debug<char name[], bool on> :: (double x) -> (); Prints output on pull; or use with debug_run(...)
int debug_init(node_t * node, const char * name, char on);

// FunGen<> :: (double t) -> (double x); Sine fn generator. Computes x = sin(t)
int fungen_init(node_t * node);

// Impulse :: (double x) -> (double d); Generates impulse fn on edges of x
int impulse_init(node_t * node);

// LPF<> :: (double x, double alpha) -> (double x_lpf); Low pass `x` with time const `alpha` (in pulls)
int lpf_init(node_t * node);
int clpf_init(node_t * node);

// Math<math_op> :: (double x[, double y]) -> (double result); Performs a basic math op on inputs (see enum math_op)
int math_init(node_t * node, enum math_op op);

// Mixer<n_channels> :: (chunk s, double vol[, chunk s2, double vol2, ...]) -> (chunk mixout); 
int mixer_init(node_t * node, size_t n_channels);
int cmixer_init(node_t * node, size_t n_channels);

// Recorder :: (chunk s, long len) -> (sample[len] samp);
int recorder_init(node_t * node);

// Sampler :: (sample samp, long cmd) -> (chunk out);
int sampler_init(node_t * node);

// Sequencer :: (double time, vector<obj> stream) -> (obj elem);
int sequencer_init(node_t * node);

// Tee<int n_inputs> :: (type val) -> (type val[, type copy, type copy, ...]); Pulls from input, duplicates
int tee_init(node_t * node, size_t n_inputs);

// Wye<int n_inputs> :: (type val[, type discard, type discard, ...]) -> (type val); Pulls from all inputs, returning the first
int wye_init(node_t * node, size_t n_inputs);

// Wave :: (double frequency, long wave_type) -> (chunk samples); See enum wave_type
int wave_init(node_t * node);

// White :: () -> (chunk samples); White noise
int white_init(node_t * node);

// Soundcard Sink :: (chunk stream) -> ();
//int soundcard_get(); // Singleton
//void soundcard_run();

#endif
