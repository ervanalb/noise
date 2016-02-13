#ifndef __NZMOD_STD_H__
#define __NZMOD_STD_H__

#include "libnoise.h"

// TYPES
extern const struct nz_typeclass nz_int_typeclass;
extern const struct nz_typeclass nz_long_typeclass;
extern const struct nz_typeclass nz_real_typeclass;
extern const struct nz_typeclass nz_chunk_typeclass;
extern const struct nz_typeclass nz_string_typeclass;
extern const struct nz_typeclass nz_array_typeclass;

// BLOCKS
extern const struct nz_blockclass nz_constant_blockclass;
extern const struct nz_blockclass nz_debug_blockclass;
extern const struct nz_blockclass nz_accumulator_blockclass;
extern const struct nz_blockclass nz_tee_blockclass;
extern const struct nz_blockclass nz_wye_blockclass;
extern const struct nz_blockclass nz_sum_blockclass;
extern const struct nz_blockclass nz_diff_blockclass;
extern const struct nz_blockclass nz_mul_blockclass;
extern const struct nz_blockclass nz_div_blockclass;
extern const struct nz_blockclass nz_mod_blockclass;
extern const struct nz_blockclass nz_pa_blockclass;
extern const struct nz_blockclass nz_mixer_blockclass;
extern const struct nz_blockclass nz_wave_blockclass;
extern const struct nz_blockclass nz_lpf_blockclass;

//audio
const struct nz_blockclass nz_envelope_blockclass;
const struct nz_blockclass nz_drum_blockclass;

//io
const struct nz_blockclass nz_wavfileout_blockclass;
int wavfileout_record(struct nz_block * block, nz_real seconds);

// --

nz_rc pa_init();
nz_rc pa_start(struct nz_block *);
void pa_term();

// MIDI
// MidiReader<filename> :: (double time) -> (array<128,midiev>); Plays an SMF MIDI file
const struct nz_blockclass nz_midireader_blockclass;

// MidiMelody<> :: (array<128,midiev>) -> (real pitch, real velocity); Extract melody from MIDI stream
const struct nz_blockclass nz_midimelody_blockclass;

// MidiDrums<> :: (array<128,midiev>) -> (real ch_0_velocity, real ch_1_velocity, ...); Extract drum events from MIDI stream
const struct nz_blockclass nz_mididrums_blockclass;

struct nz_midiev {
    unsigned char midiev_status;
    unsigned char midiev_data1;
    unsigned char midiev_data2;
};

extern const struct nz_typeclass nz_midiev_typeclass;

#endif
