#include <stdlib.h>
#include <string.h>

#include "std.h"

struct state {
    bool has_time;
    nz_real time;
    struct { 
        unsigned int needs_pull : 1;
        unsigned int is_null : 1;
        unsigned int has_start_time : 1;
        unsigned int value : 8;
        nz_real start_time;
    } velocities [128];
};

static nz_rc pull_upstream(struct nz_block self) {
    struct state * state = (struct state *) self.block_state_p;
    struct nz_midiev ev;

    state->time = 0;
    state->has_time = NZ_PULL(self, 0, &state->time) != NULL;

    for (int i = 0; i < 128; i++) {
        state->velocities[i].needs_pull = 0;
    }

    while (NZ_PULL(self, 1, &ev) != NULL) {
        /*
        char * midiev_str = NULL;
        if (nz_midiev_typeclass.type_str_obj(NULL, &ev, &midiev_str) == NZ_SUCCESS) {
            printf("Got event %s\n", midiev_str);
            free(midiev_str);
        }
        */
        //printf("drum Got event %#2x %#2x %#2x\n", ev->midiev_status, ev->midiev_data1, ev->midiev_data2);
        switch( ev.midiev_status & 0xF0 ) {
            case 0x90: ; // Note on
                if (ev.midiev_data1 >= 128) break;
                state->velocities[ev.midiev_data1].value = ev.midiev_data2;
                state->velocities[ev.midiev_data1].is_null = 0;
                state->velocities[ev.midiev_data1].has_start_time = state->has_time;
                state->velocities[ev.midiev_data1].start_time = state->time;
                //printf("Drum note on  %d %d\n", ev.midiev_data1, ev.midiev_data2);
                break;
            case 0x80: ; // Note off
                if (ev.midiev_data1 >= 128) break;
                state->velocities[ev.midiev_data1].is_null = 1;
                //printf("Drum note off %d\n", ev.midiev_data1);
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
    size_t vel_index = index / 2;

    if (state->velocities[vel_index].needs_pull) {
        nz_rc rc = pull_upstream(self);
        if (rc != NZ_SUCCESS) return NULL;
    }
    state->velocities[vel_index].needs_pull = 1;

    if (state->velocities[vel_index].is_null) {
        return NULL;
    }

    if (index % 2 == 0) { // Velocity
        *(nz_real *) obj_p = (nz_real) state->velocities[vel_index].value / 127.0;
    } else { // Time
        // How do we want to handle if there's no start time but a time now? For now return NULL; revisit later
        if (!state->has_time || !state->velocities[vel_index].has_start_time)
            return NULL;
        *(nz_real *) obj_p = state->time - state->velocities[vel_index].start_time;
    }

    return obj_p;
}

static nz_rc mididrums_block_create_args(nz_block_state ** state_pp, struct nz_block_info * info_p, const struct nz_context * context_p) {
    struct state * state = calloc(1, sizeof(struct state));
    if(state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;

    rc = nz_block_info_set_n_io(info_p, 2, 2 * 128);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_input(info_p, 0, strdup("time"), &nz_real_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_input(info_p, 1, strdup("midi"), &nz_midiev_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    for (int i = 0; i < 128; i++) {
        rc = nz_block_info_set_output(info_p, 2 * i + 0, rsprintf("vel %d", i), &nz_real_typeclass, NULL, mididrums_pull_fn);
        rc = nz_block_info_set_output(info_p, 2 * i + 1, rsprintf("time %d", i), &nz_real_typeclass, NULL, mididrums_pull_fn);
        if (rc != NZ_SUCCESS) goto fail;
        
        state->velocities[i].needs_pull = 1;
        state->velocities[i].is_null = 1;
        state->velocities[i].has_start_time = 0;
        state->velocities[i].start_time = 0;
    }

    state->has_time = false;

    *(struct state **)(state_pp) = state;
    return NZ_SUCCESS;

fail:
    free(state);
    return rc;
}

static nz_rc mididrums_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc result = arg_parse(NULL, string, NULL);
    if(result != NZ_SUCCESS) return result;

    nz_rc rc = mididrums_block_create_args(state_pp, info_p, context_p);

    return rc;
}

static void mididrums_block_destroy(nz_block_state * state_p) {
    free(state_p);
}

NZ_DECLARE_BLOCKCLASS(mididrums)
