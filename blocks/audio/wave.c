#include <stdlib.h>
#include "noise.h"
#include "types/ntypes.h"
#include "core/util.h"

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
    if(string == NULL) NZ_RETURN_ERR(NZ_EXPECTED_BLOCK_ARGS);

    const char * pos = string;
    const char * start;
    size_t length;
    int end;
    nz_rc rc;

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    rc = nz_next_block_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * shape_str = strndup(start, length);
    if(shape_str == NULL) {
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    nz_pull_fn * pull_fn;
    if(strcmp("sine", shape_str) == 0) {
        pull_fn = sine_wave_pull_fn;
    } else if(strcmp("square", shape_str) == 0) {
        pull_fn = square_wave_pull_fn;
    } else if(strcmp("saw", shape_str) == 0) {
        pull_fn = sine_wave_pull_fn;
    } else {
        char * shape_str_copy = strdup(shape_str);
        free(shape_str);
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_VALUE, shape_str_copy);
    }
    free(shape_str);

    if(pos != NULL) {
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    }

    return wave_block_create_args(pull_fn, state_pp, info_p);
}

DECLARE_BLOCKCLASS(wave)
