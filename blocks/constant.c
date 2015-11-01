#include <stdlib.h>

#include "noise.h"
#include "core/util.h"
#include "core/ntype.h"
#include "core/block.h"
#include "core/error.h"

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
    nz_rc rc;

    struct constant_block_state * state_p = calloc(1, sizeof(struct constant_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    // This function assumes ownership of typeclass_p, type_p, and obj_p
    state_p->typeclass_p = typeclass_p;
    state_p->type_p = type_p;
    state_p->obj_p = obj_p;

    if(info_p != NULL) {
        info_p->block_n_inputs = 0;
        info_p->block_n_outputs = 1;
        info_p->block_input_names = NULL;
        info_p->block_input_typeclasses = NULL;
        info_p->block_input_types = NULL;
        info_p->block_n_outputs = 1;
        info_p->block_output_names = (char *[]){(char *)"constant"};
        info_p->block_output_typeclasses = calloc(1, sizeof(const struct nz_typeclass *));
        info_p->block_output_types = calloc(1, sizeof(nz_type *));
        info_p->block_pull_fns = (nz_pull_fn *[]){constant_pull_fn};

        if(info_p->block_output_typeclasses == 0 ||
           info_p->block_output_types == 0) {
            free(info_p->block_output_typeclasses);
            free(info_p->block_output_types);
            typeclass_p->type_destroy_obj(type_p, obj_p);
            typeclass_p->type_destroy(type_p);
            free(state_p);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }

        info_p->block_output_typeclasses[0] = typeclass_p;
        info_p->block_output_types[0] = type_p;
    }

    *(struct constant_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc constant_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    return NZ_NOT_IMPLEMENTED;
}

void constant_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct constant_block_state * constant_block_state_p = (struct constant_block_state *)state_p;

    if(info_p != NULL) {
        free(info_p->block_output_typeclasses);
        free(info_p->block_output_types);
    }

    constant_block_state_p->typeclass_p->type_destroy_obj(constant_block_state_p->type_p, constant_block_state_p->obj_p);
    constant_block_state_p->typeclass_p->type_destroy(constant_block_state_p->type_p);
    free(constant_block_state_p);
}

DECLARE_BLOCKCLASS(constant)
