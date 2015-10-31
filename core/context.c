#include "core/context.h"
#include "core/error.h"
#include "core/ntype.h"
#include "core/ntypes.h"
#include "core/block.h"
#include "core/util.h"

static nz_rc nz_typesystem_init(struct nz_context * context_p) {
    context_p->context_n_typeclasses = 0;
    context_p->context_typeclass_capacity = 16;
    context_p->context_typeclasses = calloc(context_p->context_typeclass_capacity, sizeof(struct nz_typeclass *));
    if(context_p->context_typeclasses == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

nz_rc nz_context_register_typeclass(struct nz_context * context_p, struct nz_typeclass const * typeclass_p) {
    if(context_p->context_n_typeclasses < context_p->context_typeclass_capacity) {
        context_p->context_typeclass_capacity *= 2;
        struct nz_typeclass const ** newptr =  realloc(context_p->context_typeclasses, context_p->context_typeclass_capacity * sizeof(struct nz_typeclass *));
        if(newptr == NULL) {
            free(context_p->context_typeclasses);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
        context_p->context_typeclasses = newptr;
    }

    context_p->context_typeclasses[context_p->context_n_typeclasses] = typeclass_p;
    context_p->context_n_typeclasses++;

    return NZ_SUCCESS;
}

static void nz_typesystem_term(struct nz_context * context_p) {
    free(context_p->context_typeclasses);
}


static nz_rc nz_blocksystem_init(struct nz_context * context_p) {
    context_p->context_n_blockclasses = 0;
    context_p->context_blockclass_capacity = 16;
    context_p->context_blockclasses = calloc(context_p->context_blockclass_capacity, sizeof(struct nz_blockclass *));
    if(context_p->context_blockclasses == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

nz_rc nz_context_register_blockclass(struct nz_context * context_p, struct nz_blockclass const * blockclass_p) {
    if(context_p->context_n_blockclasses < context_p->context_blockclass_capacity) {
        context_p->context_blockclass_capacity *= 2;
        struct nz_blockclass const ** newptr =  realloc(context_p->context_blockclasses, context_p->context_blockclass_capacity * sizeof(struct nz_blockclass *));
        if(newptr == NULL) {
            free(context_p->context_blockclasses);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
        context_p->context_blockclasses = newptr;
    }

    context_p->context_blockclasses[context_p->context_n_blockclasses] = blockclass_p;
    context_p->context_n_blockclasses++;

    return NZ_SUCCESS;
}

static void nz_blocksystem_term(struct nz_context * context_p) {
    free(context_p->context_blockclasses);
}

nz_rc nz_context_create(struct nz_context ** context_pp) {
    nz_rc rc;
    *context_pp = malloc(sizeof(struct nz_context));
    if(!*context_pp) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    rc = nz_typesystem_init(*context_pp);
    if(rc != NZ_SUCCESS) {
        free(*context_pp);
        return rc;
    }
    rc = nz_types_init(*context_pp);
    if(rc != NZ_SUCCESS) {
        nz_typesystem_term(*context_pp);
        free(*context_pp);
        return rc;
    }
    rc = nz_blocksystem_init(*context_pp);
    if(rc != NZ_SUCCESS) {
        nz_typesystem_term(*context_pp);
        free(*context_pp);
        return rc;
    }
    rc = nz_blocks_init(*context_pp);
    if(rc != NZ_SUCCESS) {
        nz_typesystem_term(*context_pp);
        nz_blocksystem_term(*context_pp);
        free(*context_pp);
        return rc;
    }
     return NZ_SUCCESS;
}

void nz_context_destroy(struct nz_context ** context_pp) {
    nz_typesystem_term(*context_pp);
    nz_blocksystem_term(*context_pp);
    free(*context_pp);
    *context_pp = NULL;
}
