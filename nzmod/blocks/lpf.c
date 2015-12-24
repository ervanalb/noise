#include <stdlib.h>
#include <stdbool.h>

#include "std.h"

struct state {
    nz_real value;
    bool was_reset;
};

static nz_obj * lpf_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state = (struct state *) self.block_state_p;

    nz_real input = 0;
    nz_real alpha = 0;
    void * input_ptr = NZ_PULL(self, 0, &input);
    void * alpha_ptr = NZ_PULL(self, 1, &alpha);

    if (input_ptr == NULL) {
        state->was_reset = true;
        return NULL;
    }

    if (alpha_ptr == NULL || state->was_reset) {
        state->value = input;
    } else {
        state->value = alpha * input + (1. - alpha) * state->value;
    }

    state->was_reset = false;

    *(nz_real *) obj_p = state->value;
    return obj_p;
}

nz_rc lpf_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct state * state = calloc(1, sizeof(struct state));
    if(state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;
    rc = nz_block_info_set_n_io(info_p, 2, 1);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_input(info_p, 0, strdup("in"), &nz_real_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_input(info_p, 1, strdup("alpha"), &nz_real_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_output(info_p, 0, strdup("out"), &nz_real_typeclass, NULL, lpf_pull_fn);
    if (rc != NZ_SUCCESS) goto fail;

    state->value = 0.0;
    state->was_reset = true;

    *(struct state **)(state_pp) = state;
    return NZ_SUCCESS;

fail:
    nz_block_info_term(info_p);
    free(state);
    return rc;
}

void lpf_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct state * state = (struct state *)state_p;
    nz_block_info_term(info_p);
    free(state);
}

NZ_DECLARE_BLOCKCLASS(lpf);
