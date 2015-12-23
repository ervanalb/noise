#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "std.h"

static nz_real sine_wave(nz_real t) {
    return sin(t * 2 * M_PI);
}

static nz_real square_wave(nz_real t) {
    return (t >= 0.5) ? 1 : -1;
}

static nz_real saw_wave(nz_real t) {
    return 2 * t - 1;
}

static nz_obj * wave_pull_fn(nz_real (*wave)(nz_real), struct nz_block self, size_t index, nz_obj * obj_p) {
    nz_real * phase = (nz_real *)(self.block_state_p);

    nz_real freq;
    nz_real * out_chunk_p = (nz_real *)obj_p;

    if(NZ_PULL(self, 0, &freq) == NULL) return NULL;

    for(size_t i = 0; i < nz_chunk_size; i++) {
        out_chunk_p[i] = wave(*phase);
        *phase = fmod(*phase + freq / nz_frame_rate, 1.0);
    }

    return obj_p;
}

static nz_obj * sine_wave_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p)   {return wave_pull_fn(sine_wave, self, index, obj_p);}
static nz_obj * square_wave_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {return wave_pull_fn(square_wave, self, index, obj_p);}
static nz_obj * saw_wave_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p)    {return wave_pull_fn(saw_wave, self, index, obj_p);}

static nz_rc wave_block_create_args(nz_pull_fn * pull_fn, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_real * state_p = calloc(1, sizeof(nz_real));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;
    if((rc = block_info_set_n_io(info_p, 1, 1)) != NZ_SUCCESS ||
       (rc = block_info_set_input(info_p, 0, strdup("freq"), &nz_real_typeclass, NULL)) != NZ_SUCCESS ||
       (rc = block_info_set_output(info_p, 0, strdup("out"), &nz_chunk_typeclass, NULL, pull_fn)) != NZ_SUCCESS) {
        block_info_term(info_p);
        free(state_p);
        return rc;
    }

    *(nz_real **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

void wave_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    block_info_term(info_p);
    free(state_p);
}

nz_rc wave_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required generic shape", string, args);
    if(rc != NZ_SUCCESS) return rc;
    char * shape_str = (char *)args[0];

    nz_pull_fn * pull_fn;
    if(strcmp("sine", shape_str) == 0) {
        pull_fn = sine_wave_pull_fn;
    } else if(strcmp("square", shape_str) == 0) {
        pull_fn = square_wave_pull_fn;
    } else if(strcmp("saw", shape_str) == 0) {
        pull_fn = saw_wave_pull_fn;
    } else {
        // Instead of dup & free, just pass it to the error handler and let it free it
        NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, shape_str);
    }
    free(shape_str);

    return wave_block_create_args(pull_fn, state_pp, info_p);
}

DECLARE_BLOCKCLASS(wave)
