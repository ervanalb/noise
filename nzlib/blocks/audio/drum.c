#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "std.h"

static char clips_inited = 0;

struct clip {
    const char * name;
    size_t size;
    const nz_real * clip;
};

static struct clip snare_clip;
static struct clip kick_clip;
static struct clip white_clip;

static const struct clip * const clips[] = {
    &snare_clip,
    &kick_clip,
    &white_clip,
    NULL
};

struct state {
    size_t t;
    size_t clip_size;
    const nz_real * clip_start;
    nz_real last_time;
};

static nz_real rand_real() {
    return (((nz_real) rand()) / ((nz_real) RAND_MAX)) * 2. - 1.;
}

static int init_clips() {
    if (clips_inited) return 0;

    // Kick
    {
        size_t size = ((nz_frame_rate * 0.5 / nz_chunk_size) * nz_chunk_size);
        nz_real * clip = calloc(size, sizeof(nz_real));
        if(clip == NULL) goto fail;

        const nz_real dt = 1 / (nz_real) nz_frame_rate;
        const nz_real tau = exp(-8. / size);
        const nz_real f_0_tau = exp(-0.02 / size);
        nz_real f_0 = 150; // Hz

        nz_real white_tau = exp(-8. / size);
        const nz_real white_f_corner = 240; // Hz
        const nz_real white_amp = 40;

        nz_real white_alpha = 1.0 / (2. * M_PI * dt + 1.);

        nz_real t = 0;
        nz_real envelope = 1;
        nz_real white_env = 1;
        nz_real white = 0;
        for (size_t i = 0; i < size; i++) {
            white = white_alpha * white + (1.0 - white_alpha) * rand_real();
            if (white > 1.0 / white_amp) white = 1.0 / white_amp;
            if (white < -1.0 / white_amp) white = -1.0 / white_amp;

            clip[i] = sin(t * (2. * M_PI * f_0));
            clip[i] += sin(t * (2. * M_PI * f_0 * 0.75)) * 0.2;
            clip[i] += sin(t * (2. * M_PI * f_0 * 0.8)) * 0.1;
            clip[i] += sin(t * (2. * M_PI * f_0 * 0.5)) * 0.5;
            clip[i] *= 0.9;
            clip[i] += white * white_env * white_amp;
            clip[i] += rand_real() * 0.01;
            clip[i] *= envelope;

            f_0 *= f_0_tau;
            if (i > size * 0.1)
                envelope *= tau;
            if (i > size * 0.3)
                white_env *= white_tau;
            t += dt;
        }

        kick_clip.name = "kick";
        kick_clip.size = size;
        kick_clip.clip = clip;
    }

    // Snare
    { 
        size_t size = ((nz_frame_rate * 0.5 / nz_chunk_size) * nz_chunk_size);
        nz_real * clip = calloc(size, sizeof(nz_real));
        if(clip == NULL) goto fail;

        const nz_real dt = 1 / (nz_real) nz_frame_rate;
        const nz_real tau = exp(-8. / size);
        const nz_real f_0_tau = exp(-0.5 / size);
        nz_real f_0 = 120; // Hz

        nz_real white_tau = exp(-8. / size);
        const nz_real white_f_corner = 450; // Hz
        const nz_real white_amp = 150;

        nz_real white_alpha = 1.0 / (2. * M_PI * dt + 1.);

        nz_real t = 0;
        nz_real envelope = 1;
        nz_real white_env = 1;
        nz_real white = 0;
        for (size_t i = 0; i < size; i++) {
            white = white_alpha * white + (1.0 - white_alpha) * rand_real();
            if (white > 1.0 / white_amp) white = 1.0 / white_amp;
            if (white < -1.0 / white_amp) white = -1.0 / white_amp;

            clip[i] = sin(t * (2. * M_PI * f_0));
            clip[i] += sin(t * (2. * M_PI * f_0 * 1.25)) * 0.9;
            clip[i] *= 0.3;
            clip[i] += white * white_env * white_amp;
            clip[i] += rand_real() * 0.02;
            clip[i] *= envelope;

            f_0 *= f_0_tau;
            if (i > size * 0.7)
                envelope *= tau;
            if (i > size * 0.3)
                white_env *= white_tau;
            t += dt;
        }

        snare_clip.name = "snare";
        snare_clip.size = size;
        snare_clip.clip = clip;
    }

    // White
    // its actually a bit pinkish
    // wtf are percussion names
    {
        size_t size = ((nz_frame_rate * 0.5 / nz_chunk_size) * nz_chunk_size);
        nz_real * clip = calloc(size, sizeof(nz_real));
        if(clip == NULL) goto fail;

        const nz_real dt = 1 / (nz_real) nz_frame_rate;
        const nz_real tau = exp(-8. / size);

        nz_real white_tau = exp(-8. / size);
        const nz_real white_f_corner = 2000; // Hz
        const nz_real white_amp = 300; // Cool distortion

        nz_real white_alpha = 1.0 / (2. * M_PI * dt + 1.);

        nz_real t = 0;
        nz_real envelope = 1;
        nz_real white = 0;
        for (size_t i = 0; i < size; i++) {
            white = white_alpha * white + (1.0 - white_alpha) * rand_real();
            if (white > 1.0 / white_amp) white = 1.0 / white_amp;
            if (white < -1.0 / white_amp) white = -1.0 / white_amp;

            clip[i] = white * white_amp;
            clip[i] *= envelope;

            if (i > size * 0.3)
                envelope *= tau;
            t += dt;
        }

        white_clip.name = "white";
        white_clip.size = size;
        white_clip.clip = clip;

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

    nz_real time;
    bool has_time = NZ_PULL(self, 1, &time) != NULL;

    if (NZ_PULL(self, 0, &vel) == NULL || !has_time) {
        state->t = 0;
        return NULL;
    }
    if (time < state->last_time) {
        state->t = 0;
    }
    state->last_time = time;
    //printf("pull %p %ld\n", state->clip_start, state->t);

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
    if((rc = nz_block_info_set_n_io(info_p, 2, 1)) != NZ_SUCCESS ||
       (rc = nz_block_info_set_input(info_p, 0, strdup("velocity"), &nz_real_typeclass, NULL)) != NZ_SUCCESS ||
       (rc = nz_block_info_set_input(info_p, 1, strdup("time"), &nz_real_typeclass, NULL)) != NZ_SUCCESS ||
       (rc = nz_block_info_set_output(info_p, 0, strdup("out"), &nz_chunk_typeclass, NULL, drum_pull_fn)) != NZ_SUCCESS) {
        nz_block_info_term(info_p);
        free(state_p);
        return rc;
    }

    state_p->t = 0;
    state_p->clip_size = clip_size;
    state_p->clip_start = clip;

    *(struct state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

void drum_block_destroy(nz_block_state * state_p) {
    free(state_p);
}

nz_rc drum_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required generic name", string, args);
    if(rc != NZ_SUCCESS) return rc;
    char * name_str  = (char *)args[0];

    if (init_clips() != 0) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    for (size_t i = 0; clips[i] != NULL; i++) {
        const struct clip * clip = clips[i];
        if(strcmp(clip->name, name_str) == 0) {
            free(name_str);
            return drum_block_create_args(clip->clip, clip->size, state_pp, info_p);
        }
    }

    // Instead of dup & free, just pass it to the error handler and let it free it
    NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, name_str);
}

NZ_DECLARE_BLOCKCLASS(drum)
