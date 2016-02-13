#include <stdlib.h>
#include <math.h>

#include "std.h"

struct state {
    double envelope;
    double env_decay;
    nz_real input[0];
};

static nz_obj * compressor_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state = (struct state *) self.block_state_p;

    nz_real * output = (nz_real *) obj_p;

    if (NZ_PULL(self, 0, state->input) == NULL) {
        state->envelope = 0.;
        return NULL;
    }

    double chunk_max = 0.;
    size_t max_idx = nz_chunk_size - 1;
    for (size_t i = 0; i < nz_chunk_size; i++) {
        if (fabs(state->input[i]) > chunk_max) {
            chunk_max = fabs(state->input[i]);
            max_idx = i;
        }
    }

    double start_envelope = state->envelope;
    double end_envelope;
    if (chunk_max >= state->envelope) {
        end_envelope = chunk_max;
    } else {
        end_envelope = fabs(state->envelope + state->env_decay * (chunk_max - state->envelope));
    }

    for (size_t i = 0; i < nz_chunk_size; i++) {
        double env = end_envelope;
        // This algorithm is a bit shakey, because there could be a local max
        // before the chunk max that will be > than env
        // but the final clipping stage helps deal with this
        if (i < max_idx) {
            // Interpolate between start and end envelopes
            env = start_envelope + (end_envelope - start_envelope) * ((double) i / (double) max_idx);
        }
        if (env < 1.0) {
            output[i] = state->input[i];
        } else {
            output[i] = state->input[i] / env;
        }
        // Clip
        if (output[i] > 1.0) output[i] = 1.0;
        if (output[i] < -1.0) output[i] = -1.0;
    }

    state->envelope = end_envelope;

    return output;
}

void compressor_block_destroy(nz_block_state * state) {
    free(state);
}

static nz_rc compressor_block_create_args(nz_real env_decay, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct state * state = calloc(1, sizeof(*state) + nz_chunk_size * sizeof(nz_real));
    if (state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc = NZ_SUCCESS;
    rc = nz_block_info_set_n_io(info_p, 1, 1);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_output(info_p, 0, strdup("out"), &nz_chunk_typeclass, NULL, &compressor_pull_fn);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_input(info_p, 0, strdup("in"), &nz_chunk_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    state->envelope = 0;
    state->env_decay = env_decay;

    *(struct state **)(state_pp) = state;
    return NZ_SUCCESS;
fail:
    free(state);
    return rc;
}

nz_rc compressor_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required real env_decay", string, args);
    if(rc != NZ_SUCCESS) return rc;
    nz_real env_decay = *(nz_real *)args[0];
    free(args[0]);

    return compressor_block_create_args(env_decay, state_pp, info_p);
}

NZ_DECLARE_BLOCKCLASS(compressor);
