#include <stdlib.h>
#include <math.h>

#include "std.h"

nz_obj * ruler_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    nz_real * value_p = (nz_real *)(self.block_state_p);
    nz_real in;

    if(NZ_PULL(self, 0, &in) == NULL) {
        *value_p = 0;
    } else {
        *value_p = fmod(in, 1 << index);
    }

    *(nz_real *)obj_p = *value_p;

    return obj_p;
}

static nz_rc ruler_block_create_args(long n_outputs, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    if(n_outputs > 32) NZ_RETURN_ERR(NZ_ARG_VALUE);

    nz_real * state_p = calloc(1, sizeof(nz_real));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;

    if((rc = nz_block_info_set_n_io(info_p, 1, n_outputs)) != NZ_SUCCESS ||
       (rc = nz_block_info_set_input(info_p, 0, strdup("in"), &nz_real_typeclass, NULL)) != NZ_SUCCESS)
        goto fail;

    for (long out = 0; out < n_outputs; out++) {
        if ((rc = nz_block_info_set_output(info_p, out, rsprintf("out %u", 1 << out), &nz_real_typeclass, NULL, ruler_pull_fn)) != NZ_SUCCESS)
            goto fail;
    }

    *(nz_real **)(state_pp) = state_p;
    return NZ_SUCCESS;

fail:
    free(state_p);
    return rc;
}

nz_rc ruler_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required int n_outputs", string, args);
    if(rc != NZ_SUCCESS) return rc;
    long n_outputs = *(long *)args[0];
    return ruler_block_create_args(n_outputs, state_pp, info_p);
}

void ruler_block_destroy(nz_block_state * state_p) {
    free(state_p);
}

NZ_DECLARE_BLOCKCLASS(ruler)
