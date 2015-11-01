#include <stdlib.h>

#include "noise.h"
#include "core/util.h"
#include "core/ntype.h"
#include "core/block.h"
#include "core/error.h"

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
        free(state_p);
        return rc;
    }

    // This function assumes ownership of typeclass_p and type_p
    state_p->typeclass_p = typeclass_p;
    state_p->type_p = type_p;
    state_p->obj_p = obj_p;

    if(info_p != NULL) {
        info_p->block_n_inputs = 1;
        info_p->block_n_outputs = 0;
        info_p->block_input_port_array = calloc(1, sizeof(struct nz_port_info));
        info_p->block_n_outputs = 0;
        info_p->block_output_port_array = NULL;
        info_p->block_pull_fns = NULL;

        if(info_p->block_input_port_array == NULL) {
            free(info_p->block_input_port_array);
            typeclass_p->type_destroy_obj(type_p, obj_p);
            typeclass_p->type_destroy(type_p);
            free(state_p);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }

        info_p->block_input_port_array[0].block_port_name = strdup("in");
        info_p->block_input_port_array[0].block_port_typeclass_p = typeclass_p;
        info_p->block_input_port_array[0].block_port_type_p = type_p;

        if(info_p->block_input_port_array[0].block_port_name == NULL) {
            free(info_p->block_input_port_array);
            typeclass_p->type_destroy_obj(type_p, obj_p);
            typeclass_p->type_destroy(type_p);
            free(state_p);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
    }

    *(struct debug_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc debug_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
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
    char * type_str = strndup(start, length);
    rc = nz_type_create(context_p, &typeclass_p, &type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    if(pos != NULL) {
        typeclass_p->type_destroy(type_p);
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    }

    return debug_block_create_args(typeclass_p, type_p, state_pp, info_p);
}

void debug_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct debug_block_state * debug_block_state_p = (struct debug_block_state *)state_p;

    if(info_p != NULL) {
        free(info_p->block_input_port_array[0].block_port_name);
        free(info_p->block_input_port_array);
    }

    debug_block_state_p->typeclass_p->type_destroy_obj(debug_block_state_p->type_p, debug_block_state_p->obj_p);
    debug_block_state_p->typeclass_p->type_destroy(debug_block_state_p->type_p);
    free(debug_block_state_p);
}

DECLARE_BLOCKCLASS(debug)
