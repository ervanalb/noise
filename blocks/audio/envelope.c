#include <stdlib.h>

#include "noise.h"
#include "types/ntypes.h"
#include "core/argparse.h"

#include "blocks/blocks.h"

struct state {
    nz_real attack;
    nz_real release;
    nz_real value;
    nz_real last_velocity;
    enum {
        PHASE_ATTACK,
        PHASE_SUSTAIN,
        PHASE_RELEASE,
        PHASE_OFF,
    } phase;
};

static nz_obj * envelope_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state = (struct state *) self.block_state_p;

    nz_real chunk[nz_chunk_size];
    nz_real velocity = 0;
    void * chunk_ptr = NZ_PULL(self, 0, chunk);
    void * velocity_ptr = NZ_PULL(self, 1, &velocity);

    if(chunk_ptr == NULL) {
        state->value = 0;
        state->phase = PHASE_ATTACK;
        return NULL;
    }
    //TODO: refactor, omg this is bad

    if (velocity_ptr == NULL) {
        switch (state->phase) {
            case PHASE_ATTACK:
            case PHASE_SUSTAIN:
                state->phase = PHASE_RELEASE;
                break;
            case PHASE_RELEASE:
            case PHASE_OFF:
                break;
        }
        velocity = state->last_velocity;
    } else {
        state->last_velocity = velocity;
        switch (state->phase) {
            case PHASE_ATTACK:
            case PHASE_SUSTAIN:
                break;
            case PHASE_RELEASE:
            case PHASE_OFF:
                state->phase = PHASE_ATTACK;
                break;
        }
    }


    switch (state->phase) {
        case PHASE_ATTACK: ;
            // envl_attack :: seconds
            // attack_rate :: % / frame = 1 / fenvl_attack * frames_per_second)
            double attack_rate = 1.0 / (state->attack * nz_frame_rate);
            for (size_t i = 0; i < nz_chunk_size; i++) {
                chunk[i] *= state->value * velocity;
                state->value += attack_rate;
                if (state->value >= 1.0) {
                    state->phase = PHASE_SUSTAIN;
                    state->value = 1.0;
                }
            }
            break;
        case PHASE_SUSTAIN:
            for (size_t i = 0; i < nz_chunk_size; i++) {
                chunk[i] *= velocity;
            }
            break;
        case PHASE_RELEASE: ;
            double alpha = exp(- 1.0 / (nz_frame_rate * state->release));
            for (size_t i = 0; i < nz_chunk_size; i++) {
                chunk[i] *= state->value * velocity;
                state->value *= alpha;
            }
#define NZ_ENVL_CUTOFF 0.05
            if (state->value < NZ_ENVL_CUTOFF) {
                state->phase = PHASE_OFF;
                state->value = 0.0;
            }
            break;
        case PHASE_OFF:
            memset(chunk, 0, sizeof(nz_real) * nz_chunk_size);
            break;
    }

    memcpy(obj_p, chunk, sizeof(nz_real) * nz_chunk_size);
    return obj_p;
}

nz_rc envelope_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct state * state = calloc(1, sizeof(struct state));
    if(state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;
    rc = block_info_set_n_io(info_p, 2, 1);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_input(info_p, 0, strdup("chunk in"), &nz_chunk_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_input(info_p, 1, strdup("velocity in"), &nz_real_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_output(info_p, 0, strdup("out"), &nz_chunk_typeclass, NULL, envelope_pull_fn);
    if (rc != NZ_SUCCESS) goto fail;

    state->attack = 0.01;
    state->release = 0.02;
    state->value = 0.0;
    state->phase = PHASE_OFF;

    *(struct state **)(state_pp) = state;
    return NZ_SUCCESS;

fail:
    block_info_term(info_p);
    free(state);
    return rc;
}

void envelope_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct state * state = (struct state *)state_p;
    block_info_term(info_p);
    free(state);
}

DECLARE_BLOCKCLASS(envelope);
