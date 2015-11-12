#include <stdlib.h>
#include "noise.h"
#include "types/ntypes.h"
#include "core/util.h"

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
    char * n_channels_str = strndup(start, length);
    if(n_channels_str == NULL) {
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    size_t n_channels;
    if(sscanf(n_channels_str, "%lu%n", &n_channels, &end) != 1 || end <= 0 || (size_t)end != length) {
        free(n_channels_str);
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    }
    free(n_channels_str);

    if(pos != NULL) {
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    }

    return mixer_block_create_args(n_channels, state_pp, info_p);
}


DECLARE_BLOCKCLASS(mixer)
