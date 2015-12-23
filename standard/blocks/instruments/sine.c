#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"

#include "blocks/instruments/instrument.h"

struct state {
    struct nz_osc oscs[1];
    struct nz_envl envl;
};

int sine_render(void * _state, const struct nz_note * note, enum nz_instr_note_state note_state, double * chunk) {
    struct state * state = (struct state *) _state;
    
    if (note_state == NZ_INSTR_NOTE_NEW) {
        state->oscs[0].osc_freq = note->note_pitch;
        state->oscs[0].osc_amp = 1.0;

        nz_envl_init(&state->envl, 0.01, 0.02);
    }

    nz_oscbank_render(state->oscs, 1, chunk);
    return nz_envl_simple(&state->envl, note_state, chunk);
}

int nz_instrument_sine_init(struct nz_node * node) {
    int rc = nz_instr_init(node, sizeof(struct state), &sine_render);
    if (rc != 0) return rc;

    node->node_name = rsprintf("Sine Instrument");
    return 0;
}
