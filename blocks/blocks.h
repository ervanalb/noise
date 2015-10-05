#ifndef __BLOCKS_BLOCKS_H__
#define __BLOCKS_BLOCKS_H__

#include "core/block.h"
#include "blocks/audio/blocks.h"
#include "blocks/io/blocks.h"
#include "blocks/instruments/blocks.h"

enum nz_math_op {
    NZ_MATH_ADD,
    NZ_MATH_SUBTRACT,
    NZ_MATH_MULTIPLY,
    NZ_MATH_DIVIDE,
    NZ_MATH_NOTE_TO_FREQ
};

// Block defs

// Accumulator<> :: (double x) -> (double x_sum); Sums an input
int nz_accumulator_init(struct nz_node * node);

// Constant<type value> :: () -> (type value); Returns a constant value
int nz_constant_init(struct nz_node * node, struct nz_obj * constant_value);

// Debug<char name[], bool on> :: (double x) -> (); Prints output on pull; or use with debug_run(...)
int nz_debug_init(struct nz_node * node, const char * name, char on);

// Math<math_op> :: (double x[, double y]) -> (double result); Performs a basic math op on inputs (see enum math_op)
int nz_math_init(struct nz_node * node, enum nz_math_op op);

// Sequencer :: (double time, vector<obj> stream) -> (obj elem);
int nz_sequencer_init(struct nz_node * node);

// Tee<int n_inputs> :: (type val) -> (type val[, type copy, type copy, ...]); Pulls from input, duplicates
int nz_tee_init(struct nz_node * node, size_t n_inputs);

// Wye<int n_inputs> :: (type val[, type discard, type discard, ...]) -> (type val); Pulls from all inputs, returning the first
int nz_wye_init(struct nz_node * node, size_t n_inputs);

int nz_timefilter_init(struct nz_node * node);

int nz_synth_init(struct nz_node * node);


#endif
