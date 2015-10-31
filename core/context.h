#ifndef __CORE_CONTEXT_H__
#define __CORE_CONTEXT_H__

#include <stddef.h>

#include "core/ntype.h"

// --
// Public interface

nz_rc nz_context_create(struct nz_context ** context_pp);
void nz_context_destroy(struct nz_context ** context_pp);

struct nz_blockclass;
struct nz_typeclass;

nz_rc nz_blocksystem_register_blockclass(struct nz_context * context_p, struct nz_blockclass const * blockclass_p);
nz_rc nz_typesystem_register_typeclass(struct nz_context * context_p, struct nz_typeclass const * typeclass_p);

// --
// Interface for type system, block system, graph system

struct nz_context {
    // Typeclasses
    struct nz_typeclass const ** context_typeclasses;
    size_t context_n_typeclasses;
    size_t context_typeclass_capacity;

    // Blockclasses
    struct nz_blockclass const ** context_blockclasses;
    size_t context_n_blockclasses;
    size_t context_blockclass_capacity;
};

#endif
