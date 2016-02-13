#include <stdlib.h>
#include <string.h>

#include "noise.h"
#include "types/ntypes.h"
#include "core/argparse.h"

#include "blocks/io/midi.h"
#include "blocks/io/midi_smf.h"

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#define PULL_NOTE       (1 << 0)
#define PULL_VELOCITY   (1 << 1)
#define PULL_ALL        (PULL_NOTE | PULL_VELOCITY)
#define PULL_NONE       (0)

struct state {
    int pulls;
    int note;
    int velocity;
};

static nz_rc pull_upstream(struct nz_block self) {
    struct state * state = (struct state *) self.block_state_p;
    struct nz_midiev midievs[NZ_N_MIDIEVS];

    state->pulls = PULL_NONE;

    if(NZ_PULL(self, 0, &midievs) == NULL) {
        state->note = -1;
        state->velocity -1;
        return NZ_SUCCESS;
    }

    for (size_t i = 0; i < NZ_N_MIDIEVS; i++) {
        struct nz_midiev * ev = &midievs[i];
        if (ev->midiev_status == 0) break;
        printf("Got event %#2x %#2x %#2x\n", ev->midiev_status, ev->midiev_data1, ev->midiev_data2);
        switch( ev->midiev_status & 0xF0 ) {
            case 0x90: ; // Note on
                state->note = ev->midiev_data1;
                state->velocity = ev->midiev_data2;
                //printf("Note on! %d\n", state->note);
                break;
            case 0x80: ; // Note off
                if (state->note == ev->midiev_data1) {
                    // Sustain notes
                    //state->note = -1; 
                    state->velocity = -1;
                }
                break;
            //TODO: Aftertouch
            default:
                break;
        }
    }
    //printf("note = %d; velocity = %d\n", state->note, state->velocity);

    return NZ_SUCCESS;
}

static nz_obj * midimelody_pitch_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state = (struct state *) self.block_state_p;

    if (state->pulls & PULL_NOTE) {
        nz_rc rc = pull_upstream(self);
        if (rc != NZ_SUCCESS) return NULL;
    }

    state->pulls |= PULL_NOTE;

    static int i = 0;
    i++;

    if (state->note == -1) {
        printf("null pitch %d\n", i);
        return NULL;
    }

    nz_real pitch = nz_note_to_freq(state->note);
    *(nz_real *) obj_p = pitch;

    return obj_p;
}

static nz_obj * midimelody_velocity_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state = (struct state *) self.block_state_p;

    if (state->pulls & PULL_VELOCITY) {
        nz_rc rc = pull_upstream(self);
        if (rc != NZ_SUCCESS) return NULL;
    }

    state->pulls |= PULL_VELOCITY;

    if (state->velocity == -1) {
        return NULL;
    }

    nz_real velocity = ((nz_real) state->velocity) / 127.0;
    *(nz_real *) obj_p = velocity;

    return obj_p;
}


static nz_rc midimelody_block_create_args(nz_block_state ** state_pp, struct nz_block_info * info_p, const struct nz_context * context_p) {
    struct state * state = calloc(1, sizeof(struct state));
    if(state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;
    printf("array " STRINGIFY(NZ_N_MIDIEVS) "\n");

    const struct nz_typeclass * midiev_array_typeclass;
    nz_type * midiev_array_type;
    rc = nz_type_create(context_p, &midiev_array_typeclass, &midiev_array_type, "array<" STRINGIFY(NZ_N_MIDIEVS) ",midiev>");
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_n_io(info_p, 1, 2);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_input(info_p, 0, strdup("in"), midiev_array_typeclass, midiev_array_type);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_output(info_p, 0, strdup("pitch out"), &nz_real_typeclass, NULL, midimelody_pitch_pull_fn);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_output(info_p, 1, strdup("velocity out"), &nz_real_typeclass, NULL, midimelody_velocity_pull_fn);
    if (rc != NZ_SUCCESS) goto fail;

    // Open & read header of file
    state->pulls = PULL_ALL;
    state->note = -1;
    state->velocity = -1;

    *(struct state **)(state_pp) = state;
    return NZ_SUCCESS;

fail:
    block_info_term(info_p);
    free(state);
    return rc;
}

nz_rc midimelody_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc result = arg_parse(NULL, string, NULL);
    if(result != NZ_SUCCESS) return result;

    nz_rc rc = midimelody_block_create_args(state_pp, info_p, context_p);

    return rc;
}

void midimelody_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    block_info_term(info_p);
    free(state_p);
}

DECLARE_BLOCKCLASS(midimelody)
