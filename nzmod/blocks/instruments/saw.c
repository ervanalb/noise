#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"

#include "blocks/instruments/instrument.h"

#define N_SAW_HARMONICS 50

struct state {
    struct nz_osc oscs[N_SAW_HARMONICS];
    struct nz_envl envl;
};

int saw_render(void * _state, const struct nz_note * note, enum nz_instr_note_state note_state, double * chunk) {
    struct state * state = (struct state *) _state;
    
    if (note_state == NZ_INSTR_NOTE_NEW) {
        for (size_t i = 0; i < N_SAW_HARMONICS; i++) {
            state->oscs[i].osc_freq = note->note_pitch * (i + 1);
            state->oscs[i].osc_amp = pow((double) (i + 1), -1.0);
        }

        nz_envl_init(&state->envl, 0.01, 0.02);
    }

    nz_oscbank_render(state->oscs, N_SAW_HARMONICS, chunk);
    return nz_envl_simple(&state->envl, note_state, chunk);
}

int nz_instrument_saw_init(struct nz_node * node) {
    int rc = nz_instr_init(node, sizeof(struct state), &saw_render);
    if (rc != 0) return rc;

    node->node_name = rsprintf("Saw Instrument");
    return 0;
}
