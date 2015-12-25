#include <stdlib.h>
#include <stdio.h>

#include "std.h"

struct debug_block_state {
    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;
    nz_obj * obj_p;
};

nz_rc debug_pull(struct nz_block * block_p) {
    struct nz_block self = *block_p;
    struct debug_block_state * state_p = (struct debug_block_state *)(self.block_state_p);

    nz_obj * result = NZ_PULL(self, 0, state_p->obj_p);
    if(result == NULL) {
        printf("DEBUG: NULL\n");
    } else {
        char * str;
        nz_rc rc = state_p->typeclass_p->type_str_obj(state_p->type_p, result, &str);
        if(rc != NZ_SUCCESS) return rc;
        printf("DEBUG: %s\n", str);
        free(str);
    }

    return NZ_SUCCESS;
}

static nz_rc debug_block_create_args(const struct nz_typeclass * typeclass_p, nz_type * type_p, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_rc rc;

    struct debug_block_state * state_p = calloc(1, sizeof(struct debug_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_obj * obj_p;

    rc = typeclass_p->type_create_obj(type_p, &obj_p);
    if(rc != NZ_SUCCESS) {
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }

    // This function assumes ownership of typeclass_p and type_p
    state_p->typeclass_p = typeclass_p;
    state_p->type_p = type_p;
    state_p->obj_p = obj_p;

    if((rc = nz_block_info_set_n_io(info_p, 1, 0)) != NZ_SUCCESS ||
       (rc = nz_block_info_set_input(info_p, 0, strdup("in"), typeclass_p, type_p)) != NZ_SUCCESS) {
        typeclass_p->type_destroy_obj(type_p, obj_p);
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }

    *(struct debug_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc debug_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required generic type", string, args);
    if(rc != NZ_SUCCESS) return rc;
    char * type_str = (char *)args[0];

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    rc = nz_context_create_type(context_p, &typeclass_p, &type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    return debug_block_create_args(typeclass_p, type_p, state_pp, info_p);
}

void debug_block_destroy(nz_block_state * state_p) {
    struct debug_block_state * debug_block_state_p = (struct debug_block_state *)state_p;

    debug_block_state_p->typeclass_p->type_destroy_obj(debug_block_state_p->type_p, debug_block_state_p->obj_p);
    debug_block_state_p->typeclass_p->type_destroy(debug_block_state_p->type_p);
    free(debug_block_state_p);
}

NZ_DECLARE_BLOCKCLASS(debug)
