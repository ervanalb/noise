#ifndef __BLOCKS_BLOCKS_H__
#define __BLOCKS_BLOCKS_H__

#include "core/block.h"

enum nz_math_op {
    NZ_MATH_ADD,
    NZ_MATH_SUBTRACT,
    NZ_MATH_MULTIPLY,
    NZ_MATH_DIVIDE,
    NZ_MATH_NOTE_TO_FREQ
};

enum nz_wave_type {
    NZ_WAVE_SINE,
    NZ_WAVE_SAW,
    NZ_WAVE_SQUARE,
    NZ_WAVE_WHITE,
};

enum nz_sampler_command {
    // Pause: output silent chunks, keep current seek position
    NZ_SAMPLER_COMMAND_PAUSE,
    // Play: output chunks from sample until the end is reached
    NZ_SAMPLER_COMMAND_PLAY,
    // Restart: seek back to beginning of sample & start playing
    NZ_SAMPLER_COMMAND_RESTART,
    // Stop: seek back to beginning of sample & pause
    NZ_SAMPLER_COMMAND_STOP,
    // Refetch: pull in a new sample from the sample port
    NZ_SAMPLER_COMMAND_REFETCH,
};

// Block defs

// Accumulator<> :: (double x) -> (double x_sum); Sums an input
int nz_accumulator_init(struct nz_node * node);

// Constant<type value> :: () -> (type value); Returns a constant value
int nz_constant_init(struct nz_node * node, struct nz_obj * constant_value);

// Debug<char name[], bool on> :: (double x) -> (); Prints output on pull; or use with debug_run(...)
int nz_debug_init(struct nz_node * node, const char * name, char on);

// FunGen<> :: (double t) -> (double x); Sine fn generator. Computes x = sin(t)
int nz_fungen_init(struct nz_node * node);

// Impulse :: (double x) -> (double d); Generates impulse fn on edges of x
int nz_impulse_init(struct nz_node * node);

// LPF<> :: (double x, double alpha) -> (double x_lpf); Low pass `x` with time const `alpha` (in pulls)
int nz_lpf_init(struct nz_node * node);
int nz_clpf_init(struct nz_node * node);

// Math<math_op> :: (double x[, double y]) -> (double result); Performs a basic math op on inputs (see enum math_op)
int nz_math_init(struct nz_node * node, enum nz_math_op op);

// Mixer<n_channels> :: (chunk s, double vol[, chunk s2, double vol2, ...]) -> (chunk mixout); 
int nz_mixer_init(struct nz_node * node, size_t n_channels);
int nz_cmixer_init(struct nz_node * node, size_t n_channels);

// Recorder :: (chunk s, long len) -> (sample[len] samp);
int nz_recorder_init(struct nz_node * node);

// Sampler :: (sample samp, long cmd) -> (chunk out);
int nz_sampler_init(struct nz_node * node);

// Sequencer :: (double time, vector<obj> stream) -> (obj elem);
int nz_sequencer_init(struct nz_node * node);

// Tee<int n_inputs> :: (type val) -> (type val[, type copy, type copy, ...]); Pulls from input, duplicates
int nz_tee_init(struct nz_node * node, size_t n_inputs);

// Wye<int n_inputs> :: (type val[, type discard, type discard, ...]) -> (type val); Pulls from all inputs, returning the first
int nz_wye_init(struct nz_node * node, size_t n_inputs);

// Wave :: (double frequency, long wave_type) -> (chunk samples); See enum wave_type
int nz_wave_init(struct nz_node * node);

// White :: () -> (chunk samples); White noise
int nz_white_init(struct nz_node * node);

// Soundcard Sink :: (chunk stream) -> ();
//int soundcard_get(); // Singleton
//void soundcard_run();

#endif
