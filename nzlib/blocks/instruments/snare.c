#include <math.h>
#include <stdlib.h>

#include "noise.h"
#include "blocks/blocks.h"
#include "core/util.h"

#include "blocks/instruments/instrument.h"

struct state {
    struct nz_envl envl;
};

int render(void * _state, const struct nz_note * note, enum nz_instr_note_state note_state, double * chunk) {
    struct state * state = (struct state *) _state;
    
    if (note_state == NZ_INSTR_NOTE_NEW) {
        nz_envl_init(&state->envl, 0.01, 0.02);
    }

    for (size_t i = 0; i < nz_chunk_size; i++) {
        chunk[i] = (rand() / (double) (RAND_MAX / 2)) - 1.0; 
    }

    return nz_envl_simple(&state->envl, note_state, chunk);
}

int nz_instrument_snare_init(struct nz_node * node) {
    int rc = nz_instr_init(node, sizeof(struct state), &render);
    if (rc != 0) return rc;

    node->node_name = rsprintf("Snare Instrument");
    return 0;
}
