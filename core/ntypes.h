#ifndef __CORE_NTYPES_H__
#define __CORE_NTYPES_H__

#include "core/error.h"

struct nz_context;

const struct nz_typeclass nz_int_typeclass;
const struct nz_typeclass nz_long_typeclass;
const struct nz_typeclass nz_real_typeclass;
const struct nz_typeclass nz_chunk_typeclass;
const struct nz_typeclass nz_string_typeclass;
const struct nz_typeclass nz_array_typeclass;
 
nz_rc nz_types_init(struct nz_context * context);

#endif
