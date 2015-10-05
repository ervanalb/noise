#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "blocks/instruments/instrument.h"
#include "core/util.h"

struct state {
    double phase;
    double envelope;

    struct nz_osc oscbank[4];
    //double env_decay;
};

static int synth_render(void * _state, const struct nz_note * note, enum nz_instr_note_state note_state, double * chunk) {
    struct state * state = (struct state *) _state;
    double env_decay = 1.0;
    double env_attack = 0.0;

    switch (note_state) {
        case NZ_INSTR_NOTE_NEW:
            memset(state, 0, sizeof(*state));
            state->phase = 0;
            state->envelope = 0.;
            env_attack = 0.01;
            break;
        case NZ_INSTR_NOTE_ON: 
            env_attack = 0.01;
            break;
        case NZ_INSTR_NOTE_OFF:
            if (state->envelope < 0.01) return -1;
            env_decay = 0.999;
            break;
        default: return -1;
    }

    double freq = nz_note_to_freq(note->note_pitch);

    state->oscbank[0].osc_freq = freq * 1.0;
    state->oscbank[0].osc_amp = 1.0;
    state->oscbank[1].osc_freq = freq * 2.0;
    state->oscbank[1].osc_amp = 1 / 2.;
    state->oscbank[2].osc_freq = freq * 3.0;
    state->oscbank[2].osc_amp = 1 / 3.;
    state->oscbank[3].osc_freq = freq * 4.0;
    state->oscbank[3].osc_amp = 1 / 4.;

    nz_oscbank_render(state->oscbank, 4, chunk);

    for (size_t i = 0; i < nz_chunk_size; i++) {
        chunk[i] *= state->envelope;
        if (env_attack > 0) {
            state->envelope = fmin(1.0, state->envelope + env_attack);
        } else {
            state->envelope *= env_decay;
        }
    }

    return 0;
}

int nz_synth_init(struct nz_node * node) {
    nz_instrument_init(node, sizeof(struct state), &synth_render);
    node->node_name = strdup("Instrument");
    return 0;
}

