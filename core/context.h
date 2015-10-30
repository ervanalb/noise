#ifndef __CORE_CONTEXT_H__
#define __CORE_CONTEXT_H__

#include <stddef.h>

#include "core/ntype.h"

// --
// Public interface

nz_rc nz_create_context(struct nz_context ** context_pp);
void nz_destroy_context(struct nz_context * context_p);

// --
// Interface for type system, block system, graph system

struct nz_context {
    // Typeclasses
    struct nz_typeclass const ** registered_typeclasses;
    size_t n_registered_typeclasses;
    size_t registered_typeclass_capacity;

    // Blockclasses
    struct nz_blockclass const ** registered_blockclasses;
    size_t n_registered_blockclasses;
    size_t registered_blockclass_capacity;
};

#endif
