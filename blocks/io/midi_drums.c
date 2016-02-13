#include <stdlib.h>
#include <string.h>

#include "noise.h"
#include "types/ntypes.h"
#include "core/argparse.h"

#include "blocks/io/midi.h"
#include "blocks/io/midi_smf.h"

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

struct state {
    struct { 
        unsigned int needs_pull : 1;
        unsigned int is_null : 1;
        unsigned int value : 8;
    } velocities [128];
};

static nz_rc pull_upstream(struct nz_block self) {
    struct state * state = (struct state *) self.block_state_p;
    struct nz_midiev midievs[NZ_N_MIDIEVS];

    for (int i = 0; i < 128; i++) {
        state->velocities[i].needs_pull = 0;
    }

    if(NZ_PULL(self, 0, &midievs) == NULL) {
        for (int i = 0; i < 128; i++) {
            state->velocities[i].is_null = 1;
        }
        printf("drum upstream null\n");
        return NZ_SUCCESS;
    }

    for (size_t i = 0; i < NZ_N_MIDIEVS; i++) {
        struct nz_midiev * ev = &midievs[i];
        if (ev->midiev_status == 0) break;
        printf("drum Got event %#2x %#2x %#2x\n", ev->midiev_status, ev->midiev_data1, ev->midiev_data2);
        switch( ev->midiev_status & 0xF0 ) {
            case 0x90: ; // Note on
                if (ev->midiev_data1 >= 128) break;
                state->velocities[ev->midiev_data1].value = ev->midiev_data2;
                state->velocities[ev->midiev_data1].is_null = 0;
                printf("Drum note on  %d %d\n", ev->midiev_data1, ev->midiev_data2);
                break;
            case 0x80: ; // Note off
                if (ev->midiev_data1 >= 128) break;
                state->velocities[ev->midiev_data1].is_null = 1;
                printf("Drum note off %d\n", ev->midiev_data1);
                break;
            //TODO: Aftertouch
            default:
                break;
        }
    }
    //printf("note = %d; velocity = %d\n", state->note, state->velocity);

    return NZ_SUCCESS;
}

static nz_obj * mididrums_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state = (struct state *) self.block_state_p;

    printf("needs pull %ld %d\n" ,index, state->velocities[index].needs_pull);
    if (state->velocities[index].needs_pull) {
        printf("pulling...\n");
        nz_rc rc = pull_upstream(self);
        if (rc != NZ_SUCCESS) return NULL;
    }

    state->velocities[index].needs_pull = 1;

    if (state->velocities[index].is_null) {
        printf("drum isnull %ld\n", index);
        return NULL;
    }
    printf("drum on %ld\n", index);

    *(nz_real *) obj_p = (nz_real) state->velocities[index].value / 127.0;

    return obj_p;
}

static nz_rc mididrums_block_create_args(nz_block_state ** state_pp, struct nz_block_info * info_p, const struct nz_context * context_p) {
    struct state * state = calloc(1, sizeof(struct state));
    if(state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;

    const struct nz_typeclass * midiev_array_typeclass;
    nz_type * midiev_array_type;
    rc = nz_type_create(context_p, &midiev_array_typeclass, &midiev_array_type, "array<" STRINGIFY(NZ_N_MIDIEVS) ",midiev>");
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_n_io(info_p, 1, 128);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_input(info_p, 0, strdup("in"), midiev_array_typeclass, midiev_array_type);
    if (rc != NZ_SUCCESS) goto fail;

    for (int i = 0; i < 128; i++) {
        rc = block_info_set_output(info_p, i, rsprintf("out %d", i), &nz_real_typeclass, NULL, mididrums_pull_fn);
        if (rc != NZ_SUCCESS) goto fail;
        
        state->velocities[i].needs_pull = 1;
        state->velocities[i].is_null = 1;
    }

    *(struct state **)(state_pp) = state;
    return NZ_SUCCESS;

fail:
    block_info_term(info_p);
    free(state);
    return rc;
}

static nz_rc mididrums_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc result = arg_parse(NULL, string, NULL);
    if(result != NZ_SUCCESS) return result;

    nz_rc rc = mididrums_block_create_args(state_pp, info_p, context_p);

    return rc;
}

static void mididrums_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    block_info_term(info_p);
    free(state_p);
}

DECLARE_BLOCKCLASS(mididrums)
