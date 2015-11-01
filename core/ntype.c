#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "noise.h"
#include "core/context.h"
#include "core/ntype.h"
#include "core/util.h"

int nz_types_are_equal(const struct nz_typeclass * typeclass_p,       const nz_type * type_p,
                       const struct nz_typeclass * other_typeclass_p, const nz_type * other_type_p)
{
    if(strcmp(typeclass_p->type_id, other_typeclass_p->type_id)) return 0;
    return typeclass_p->type_is_equal(type_p, other_type_p);
}

nz_rc nz_type_create(const struct nz_context * context_p, const struct nz_typeclass ** typeclass_pp, nz_type ** type_pp, const char * string) {
    for(size_t i = 0; i < context_p->context_n_typeclasses; i++) {
        size_t c = 0;
        const char * type_id = context_p->context_typeclasses[i]->type_id;
        size_t len = strlen(string);
        for(;;) {
            if(type_id[c] == '\0') {
                if(string[c] == '\0') {
                    // Match, no args
                    *typeclass_pp = context_p->context_typeclasses[i];
                    return (*typeclass_pp)->type_create(context_p, type_pp, NULL);
                } else if(string[c] == '<' && string[len - 1] == '>') {
                    // Match, args
                    char * args = strndup(&(string[c + 1]), len - 2 - c);
                    if(args == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                    *typeclass_pp = context_p->context_typeclasses[i];
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
