#include <stdlib.h>
#include "noise.h"
#include "types/ntypes.h"
#include "core/util.h"
#include "core/argparse.h"

static char clips_inited = 0;
static size_t snare_clip_size;
static const nz_real * snare_clip = NULL;

static size_t kick_clip_size;
static const nz_real * kick_clip = NULL;

struct state {
    size_t t;
    size_t clip_size;
    const nz_real * clip_start;
};

static nz_real rand_real() {
    return (((nz_real) rand()) / ((nz_real) RAND_MAX)) * 2. - 1.;
}

static int init_clips() {
    if (clips_inited) return 0;

    // Snare
    {
        snare_clip_size = ((nz_frame_rate * 0.5 / nz_chunk_size) * nz_chunk_size);
        nz_real * clip = calloc(snare_clip_size, sizeof(*snare_clip));
        if(clip == NULL) goto fail;

        const nz_real tau = exp(-8. / snare_clip_size);

        nz_real t = 0;
        nz_real envelope = 1;
        for (size_t i = 0; i < snare_clip_size; i++) {
            clip[i] = rand_real() * envelope;

            envelope *= tau;
            t += 1 / (nz_real) nz_frame_rate;
        }

        snare_clip = clip;
    }

    // Kick
    { 
        kick_clip_size = ((nz_frame_rate * 0.5 / nz_chunk_size) * nz_chunk_size);
        nz_real * clip = calloc(kick_clip_size, sizeof(*kick_clip));
        if(clip == NULL) goto fail;

        const nz_real tau = exp(-8. / kick_clip_size);
        const nz_real f_0 = 60; // Hz

        nz_real t = 0;
        nz_real envelope = 1;
        for (size_t i = 0; i < kick_clip_size; i++) {
            clip[i] = sin(t * (2. * M_PI * f_0));
            clip[i] += sin(t * (2. * M_PI * f_0 * 1.25)) * 0.9;
            clip[i] += sin(t * (2. * M_PI * f_0 * 1.33)) * 0.8;
            clip[i] += sin(t * (2. * M_PI * f_0 * 2.0)) * 0.7;
            clip[i] *= envelope * .6;

            envelope *= tau;
            t += 1 / (nz_real) nz_frame_rate;
        }

        kick_clip = clip;
    }

    clips_inited = 1;
    return 0;

fail:
    clips_inited = 0;
    return -1;
}

static nz_obj * drum_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state  = (struct state *)(self.block_state_p);

    nz_real * out_chunk_p = (nz_real *)obj_p;
    nz_real vel = 0;

    if (NZ_PULL(self, 0, &vel) == NULL) {
        state->t = 0;
        return NULL;
    }
    printf("pull %p %ld\n", state->clip_start, state->t);

    if (state->t + nz_chunk_size > state->clip_size) {
        return NULL;
    }

    memcpy(out_chunk_p, state->clip_start + state->t, sizeof(nz_real) * nz_chunk_size);
    state->t += nz_chunk_size;

    return obj_p;
}

static nz_rc drum_block_create_args(const nz_real * clip, size_t clip_size, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct state * state_p = calloc(1, sizeof(*state_p));
    if (state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    if (init_clips() != 0) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;
    if((rc = block_info_set_n_io(info_p, 1, 1)) != NZ_SUCCESS ||
       (rc = block_info_set_input(info_p, 0, strdup("velocity"), &nz_real_typeclass, NULL)) != NZ_SUCCESS ||
       (rc = block_info_set_output(info_p, 0, strdup("out"), &nz_chunk_typeclass, NULL, drum_pull_fn)) != NZ_SUCCESS) {
        block_info_term(info_p);
        free(state_p);
        return rc;
    }

    state_p->t = 0;
    state_p->clip_size = clip_size;
    state_p->clip_start = clip;

    *(struct state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

void drum_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    block_info_term(info_p);
    free(state_p);
}

nz_rc drum_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required generic name", string, args);
    if(rc != NZ_SUCCESS) return rc;
    char * name_str  = (char *)args[0];

    if (init_clips() != 0) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    if(strcmp("kick", name_str) == 0) {
        free(name_str);
        return drum_block_create_args(kick_clip, kick_clip_size, state_pp, info_p);
    } else if(strcmp("snare", name_str) == 0) {
        free(name_str);
        return drum_block_create_args(snare_clip, snare_clip_size, state_pp, info_p);
    } else {
        // Instead of dup & free, just pass it to the error handler and let it free it
        NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, name_str);
    }
}

DECLARE_BLOCKCLASS(drum)
