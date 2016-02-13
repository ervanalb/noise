#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "libnoise.h"

int nz_types_are_equal(const struct nz_typeclass * typeclass_p,       const nz_type * type_p,
                       const struct nz_typeclass * other_typeclass_p, const nz_type * other_type_p)
{
    if(strcmp(typeclass_p->type_id, other_typeclass_p->type_id)) return 0;
    return typeclass_p->type_is_equal(type_p, other_type_p);
}
