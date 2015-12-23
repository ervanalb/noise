#include <stdlib.h>
#include "noise.h"
#include "types/std.h"
#include "core/argparse.h"

nz_obj * accumulator_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    nz_real * accumulator_value_p = (nz_real *)(self.block_state_p);
    nz_real in;

    if(NZ_PULL(self, 0, &in) == NULL) {
        *accumulator_value_p = 0;
    } else {
        *accumulator_value_p += in;
    }

    *(nz_real *)obj_p = *accumulator_value_p;

    return obj_p;
}

static nz_rc accumulator_block_create_args(nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_real * state_p = calloc(1, sizeof(nz_real));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;
    if((rc = block_info_set_n_io(info_p, 1, 1)) != NZ_SUCCESS ||
       (rc = block_info_set_input(info_p, 0, strdup("in"), &nz_real_typeclass, NULL)) != NZ_SUCCESS ||
       (rc = block_info_set_output(info_p, 0, strdup("out"), &nz_real_typeclass, NULL, accumulator_pull_fn)) != NZ_SUCCESS) {
        block_info_term(info_p);
        free(state_p);
        return rc;
    }

    *(nz_real **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc accumulator_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc result = arg_parse(NULL, string, NULL);
    if(result != NZ_SUCCESS) return result;
    return accumulator_block_create_args(state_pp, info_p);
}

void accumulator_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    block_info_term(info_p);
    free(state_p);
}

DECLARE_BLOCKCLASS(accumulator)
