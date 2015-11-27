#include <stdlib.h>
#include "noise.h"
#include "types/ntypes.h"
#include "core/argparse.h"

nz_obj * sum_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    nz_real * math_value_p = (nz_real *)(self.block_state_p);
    nz_real a;
    nz_real b;

    if((NZ_PULL(self, 0, &a) == NULL) | (NZ_PULL(self, 1, &b) == NULL)) return NULL;

    *(nz_real *)obj_p = a + b;

    return obj_p;
}

nz_obj * diff_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    nz_real * math_value_p = (nz_real *)(self.block_state_p);
    nz_real a;
    nz_real b;

    if((NZ_PULL(self, 0, &a) == NULL) | (NZ_PULL(self, 1, &b) == NULL)) return NULL;

    *(nz_real *)obj_p = a - b;

    return obj_p;
}

nz_obj * mul_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    nz_real * math_value_p = (nz_real *)(self.block_state_p);
    nz_real a;
    nz_real b;

    if((NZ_PULL(self, 0, &a) == NULL) | (NZ_PULL(self, 1, &b) == NULL)) return NULL;

    *(nz_real *)obj_p = a * b;

    return obj_p;
}

nz_obj * div_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    nz_real * math_value_p = (nz_real *)(self.block_state_p);
    nz_real a;
    nz_real b;

    if((NZ_PULL(self, 0, &a) == NULL) | (NZ_PULL(self, 1, &b) == NULL || b == 0)) return NULL;

    *(nz_real *)obj_p = a / b;

    return obj_p;
}

static nz_rc math_block_create_args(nz_pull_fn * pull_fn, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_real * state_p = calloc(1, sizeof(nz_real));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;
    if((rc = block_info_set_n_io(info_p, 1, 1)) != NZ_SUCCESS ||
       (rc = block_info_set_input(info_p, 0, strdup("a"), &nz_real_typeclass, NULL)) != NZ_SUCCESS ||
       (rc = block_info_set_input(info_p, 1, strdup("b"), &nz_real_typeclass, NULL)) != NZ_SUCCESS ||
       (rc = block_info_set_output(info_p, 0, strdup("out"), &nz_real_typeclass, NULL, pull_fn)) != NZ_SUCCESS) {
        block_info_term(info_p);
        free(state_p);
        return rc;
    }

    *(nz_real **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc sum_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc result = arg_parse(NULL, string, NULL);
    if(result != NZ_SUCCESS) return result;
    return math_block_create_args(sum_pull_fn, state_pp, info_p);
}

nz_rc diff_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc result = arg_parse(NULL, string, NULL);
    if(result != NZ_SUCCESS) return result;
    return math_block_create_args(diff_pull_fn, state_pp, info_p);
}

nz_rc mul_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc result = arg_parse(NULL, string, NULL);
    if(result != NZ_SUCCESS) return result;
    return math_block_create_args(mul_pull_fn, state_pp, info_p);
}

nz_rc div_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc result = arg_parse(NULL, string, NULL);
    if(result != NZ_SUCCESS) return result;
    return math_block_create_args(div_pull_fn, state_pp, info_p);
}

void math_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    block_info_term(info_p);
    free(state_p);
}

void sum_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {math_block_destroy(state_p, info_p);}
void diff_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {math_block_destroy(state_p, info_p);}
void mul_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {math_block_destroy(state_p, info_p);}
void div_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {math_block_destroy(state_p, info_p);}

DECLARE_BLOCKCLASS(sum)
DECLARE_BLOCKCLASS(diff)
DECLARE_BLOCKCLASS(mul)
DECLARE_BLOCKCLASS(div)
