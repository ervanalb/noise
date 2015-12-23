#include <stdlib.h>

#include "noise.h"
#include "core/util.h"
#include "core/ntype.h"
#include "core/block.h"
#include "core/error.h"
#include "core/argparse.h"

struct constant_block_state {
    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;
    nz_obj * obj_p;
};

nz_obj * constant_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct constant_block_state * constant_block_state_p = (struct constant_block_state *)(self.block_state_p);
    nz_rc rc;

    rc = constant_block_state_p->typeclass_p->type_copy_obj(constant_block_state_p->type_p, obj_p, constant_block_state_p->obj_p);
    if(rc == NZ_SUCCESS) return obj_p;

    // TODO THROW PULL-TIME ERROR HERE

    return NULL;
}

static nz_rc constant_block_create_args(const struct nz_typeclass * typeclass_p, nz_type * type_p, nz_obj * obj_p, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct constant_block_state * state_p = calloc(1, sizeof(struct constant_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    // This function assumes ownership of typeclass_p, type_p, and obj_p
    state_p->typeclass_p = typeclass_p;
    state_p->type_p = type_p;
    state_p->obj_p = obj_p;

    nz_rc rc;
    if((rc = block_info_set_n_io(info_p, 0, 1)) != NZ_SUCCESS ||
       (rc = block_info_set_output(info_p, 0, strdup("out"), typeclass_p, type_p, constant_pull_fn)) != NZ_SUCCESS) {
        block_info_term(info_p);
        typeclass_p->type_destroy_obj(type_p, obj_p);
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }

    *(struct constant_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc constant_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[2];
    nz_rc rc = arg_parse("required generic type, required generic value", string, args);
    if(rc != NZ_SUCCESS) return rc;
    char * type_str = (char *)args[0];
    char * value_str = (char *)args[1];

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;
    nz_obj * obj_p;

    rc = nz_type_create(context_p, &typeclass_p, &type_p, type_str);
    if(rc != NZ_SUCCESS) {
        free(type_str);
        free(value_str);
        return rc;
    }

    rc = typeclass_p->type_create_obj(type_p, &obj_p);
    if(rc != NZ_SUCCESS) {
        free(type_str);
        free(value_str);
        typeclass_p->type_destroy(type_p);
        return rc;
    }

    rc = typeclass_p->type_init_obj(type_p, obj_p, value_str);
    if(rc != NZ_SUCCESS) {
        free(type_str);
        free(value_str);
        typeclass_p->type_destroy_obj(type_p, obj_p);
        typeclass_p->type_destroy(type_p);
        return rc;
    }

    free(type_str);
    free(value_str);

    return constant_block_create_args(typeclass_p, type_p, obj_p, state_pp, info_p);
}

void constant_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct constant_block_state * constant_block_state_p = (struct constant_block_state *)state_p;

    block_info_term(info_p);
    constant_block_state_p->typeclass_p->type_destroy_obj(constant_block_state_p->type_p, constant_block_state_p->obj_p);
    constant_block_state_p->typeclass_p->type_destroy(constant_block_state_p->type_p);
    free(constant_block_state_p);
}

DECLARE_BLOCKCLASS(constant)
