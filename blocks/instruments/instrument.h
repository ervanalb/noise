#ifndef __BLOCKS_INSTRUMENT_H__
#define __BLOCKS_INSTRUMENT_H__

#include "noise.h"

// Instrument helper
// Designed for making blocks which take in lists of notes and
// render each one separatrely, abstracting away polyphony

enum nz_instr_note_state {
    NZ_INSTR_NOTE_NEW,
    NZ_INSTR_NOTE_ON,
    NZ_INSTR_NOTE_OFF,
};

typedef int (*nz_instr_render_fpt)(void * state, const struct nz_note * note, enum nz_instr_note_state note_state, double * chunk);

int nz_instrument_init(struct nz_node * node, size_t state_size, nz_instr_render_fpt render);


// Oscillator bank + simple envelope helpers
struct nz_osc {
    double osc_amp;
    double osc_freq;
    double osc_phase;
};

void nz_oscbank_render(struct nz_osc * oscs, size_t n_oscs, double * chunk);

// Envelope with linear attack and exponential decay
enum nz_envl_state { 
    NZ_ENVL_ATTACK,
    NZ_ENVL_SUSTAIN,
    NZ_ENVL_DECAY,
    NZ_ENVL_OFF,
};

#define NZ_ENVL_CUTOFF 0.01

struct nz_envl {
    double envl_attack; // linear factor in "percent per second"
    double envl_decay; // time constant in "per second" (seconds to decay to 1/e of original value)
    enum nz_envl_state envl_state;
    double envl_value;
};

int nz_envl_simple(struct nz_envl * envl, enum nz_instr_note_state note_state, double * chunk);

// --- Actual instrument blocks  ---

// Basic sine wave instrument
int nz_instrument_sine_init(struct nz_node * node); 
int nz_instrument_saw_init(struct nz_node * node);

#endif
