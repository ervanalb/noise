#ifndef __SYNTH_H
#define __SYNTH_H

#include "block.h"

typedef struct {
    double attack_t;
    double attack_amp;
    double decay_t;
    double release_t;

    int num_notes;
} synth_info_t;

typedef struct {
    int active;
    int note;
    double velocity;
    double start_t;
    double stop_t;
} synth_note_t;

typedef struct {
	double t;

    double attack_t;
    double attack_amp;
    double decay_t;
    double release_t;

	double* chunk;

    int num_notes;
    synth_note_t* notes;
} synth_state_t;

error_t synth_state_alloc(block_info_pt block_info, state_pt* state);
void synth_state_free(block_info_pt block_info, state_pt state);
error_t synth_state_copy(block_info_pt block_info, state_pt dest, state_pt source);

error_t synth_pull(node_t* node, output_pt* output);

#endif
