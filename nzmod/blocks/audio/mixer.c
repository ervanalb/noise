#include <stdlib.h>

#include "std.h"

struct mixer_block_state {
    size_t n_channels;
    nz_real * in_chunk_p;
};

static nz_obj * mixer_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct mixer_block_state * state_p = (struct mixer_block_state *)(self.block_state_p);

    nz_real gain;
    nz_real * out_chunk_p = (nz_real *)obj_p;

    for(size_t i = 0; i < nz_chunk_size; i++) out_chunk_p[i] = 0;

    for(size_t i = 0; i < state_p->n_channels; i++) {
        if((NZ_PULL(self, i * 2, state_p->in_chunk_p) != NULL) & (NZ_PULL(self, i * 2 + 1, &gain) != NULL)) {
            for(size_t j = 0; j < nz_chunk_size; j++) out_chunk_p[j] += gain * state_p->in_chunk_p[j];
        }
    }

    return obj_p;
}

static nz_rc mixer_block_create_args(size_t n_channels, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct mixer_block_state * state_p = calloc(1, sizeof(struct mixer_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    state_p->n_channels = n_channels;

    state_p->in_chunk_p = calloc(nz_chunk_size, sizeof(nz_real));
    if(state_p->in_chunk_p == NULL) {
        free(state_p);
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    nz_rc rc;
    if((rc = block_info_set_n_io(info_p, 2 * n_channels, 1)) == NZ_SUCCESS &&
       (rc = block_info_set_output(info_p, 0, strdup("out"), &nz_chunk_typeclass, NULL, &mixer_pull_fn)) == NZ_SUCCESS) {

        for(size_t i = 0; i < n_channels; i++) {
           if((rc = block_info_set_input(info_p, i * 2, rsprintf("in %lu", i + 1), &nz_chunk_typeclass, NULL)) != NZ_SUCCESS ||
              (rc = block_info_set_input(info_p, i * 2 + 1, rsprintf("gain %lu", i + 1), &nz_real_typeclass, NULL)) != NZ_SUCCESS) break;
        }
    }

    if(rc != NZ_SUCCESS) {
        block_info_term(info_p);
        free(state_p->in_chunk_p);
        free(state_p);
        return rc;
    }

    *(struct mixer_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

void mixer_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct mixer_block_state * mixer_block_state_p = (struct mixer_block_state *)state_p;
    block_info_term(info_p);
    free(mixer_block_state_p->in_chunk_p);
    free(mixer_block_state_p);
}

nz_rc mixer_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required int n_channels", string, args);
    if(rc != NZ_SUCCESS) return rc;
    long n_channels = *(long *)args[0];
    free(args[0]);

    if(n_channels < 0) NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, rsprintf("%ld", n_channels));

    return mixer_block_create_args(n_channels, state_pp, info_p);
}


DECLARE_BLOCKCLASS(mixer)
