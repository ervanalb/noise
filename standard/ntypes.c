#include "noise.h"
#include "core/context.h"

#include "ntypes.h"
#include "types/std.h"

nz_rc nz_types_init(struct nz_context * context_p) {
    nz_rc rc;
    rc = nz_context_register_typeclass(context_p, &nz_int_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_long_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_real_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_chunk_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_string_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_array_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_typeclass(context_p, &nz_midiev_typeclass); if(rc != NZ_SUCCESS) return rc;
    return NZ_SUCCESS;
}
