#ifndef __BLOCKS_INSTRUMENT_H__
#define __BLOCKS_INSTRUMENT_H__

#include "noise.h"

enum nz_instr_note_state {
    NZ_INSTR_NOTE_NEW,
    NZ_INSTR_NOTE_ON,
    NZ_INSTR_NOTE_OFF,
};

typedef int (*nz_instr_render_fpt)(void * state, const struct nz_note * note, enum nz_instr_note_state note_state, double * chunk);

int nz_instrument_init(struct nz_node * node, size_t state_size, nz_instr_render_fpt render);


struct nz_osc {
    double osc_amp;
    double osc_freq;
    double osc_phase;
};

void nz_oscbank_render(struct nz_osc * oscs, size_t n_oscs, double * chunk);
int nz_sine_render(void * _state, const struct nz_note * note, enum nz_instr_note_state note_state, double * chunk);
extern size_t nz_sine_state_size;

#endif
