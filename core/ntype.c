#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "noise.h"
#include "core/context.h"
#include "core/ntype.h"
#include "core/util.h"

nz_rc nz_init_type_system(struct nz_context * context_p) {
    context_p->n_registered_typeclasses = 0;
    context_p->registered_typeclass_capacity = 16;
    context_p->registered_typeclasses = calloc(context_p->registered_typeclass_capacity, sizeof(struct nz_typeclass *));
    if(context_p->registered_typeclasses == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

nz_rc nz_register_typeclass(struct nz_context * context_p, struct nz_typeclass const * typeclass_p) {
    if(context_p->n_registered_typeclasses < context_p->registered_typeclass_capacity) {
        context_p->registered_typeclass_capacity *= 2;
        struct nz_typeclass const ** newptr =  realloc(context_p->registered_typeclasses, context_p->registered_typeclass_capacity * sizeof(struct nz_typeclass *));
        if(newptr == NULL) {
            free(context_p->registered_typeclasses);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
        context_p->registered_typeclasses = newptr;
    }

    context_p->registered_typeclasses[context_p->n_registered_typeclasses] = typeclass_p;
    context_p->n_registered_typeclasses++;

    return NZ_SUCCESS;
}

void nz_deinit_type_system(struct nz_context * context_p) {
    free(context_p->registered_typeclasses);
}

int nz_types_are_equal(const struct nz_typeclass * typeclass_p,       const nz_type * type_p,
                       const struct nz_typeclass * other_typeclass_p, const nz_type * other_type_p)
{
    if(strcmp(typeclass_p->type_id, other_typeclass_p->type_id)) return 0;
    return typeclass_p->type_is_equal(type_p, other_type_p);
}

nz_rc nz_type_create(const struct nz_context * context_p, const struct nz_typeclass ** typeclass_pp, nz_type ** type_pp, const char * string) {
    for(size_t i = 0; i < context_p->n_registered_typeclasses; i++) {
        size_t c = 0;
        const char * type_id = context_p->registered_typeclasses[i]->type_id;
        size_t len = strlen(string);
        for(;;) {
            if(type_id[c] == '\0') {
                if(string[c] == '\0') {
                    // Match, no args
                    *typeclass_pp = context_p->registered_typeclasses[i];
                    return (*typeclass_pp)->type_create(context_p, type_pp, NULL);
                } else if(string[c] == '<' && string[len - 1] == '>') {
                    // Match, args
                    char * args = strndup(&(string[c + 1]), len - 2 - c);
                    if(args == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                    *typeclass_pp = context_p->registered_typeclasses[i];
                    nz_rc rc = (*typeclass_pp)->type_create(context_p, type_pp, args);
                    free(args);
                    return rc;
                } else {
                    break; // No match, type ended early
                }
            } else if(string[c] == '\0') {
                break; // No match, string ended early
            } else if(string[c] == type_id[c]) {
                c++;
            } else {
                break;
            }
        }
    }

    NZ_RETURN_ERR(NZ_TYPE_NOT_FOUND);
}


