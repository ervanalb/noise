#ifndef __CORE_NTYPES_H__
#define __CORE_NTYPES_H__

#include "core/error.h"
#include "types/midi.h"

struct nz_context;

extern const struct nz_typeclass nz_int_typeclass;
extern const struct nz_typeclass nz_long_typeclass;
extern const struct nz_typeclass nz_real_typeclass;
extern const struct nz_typeclass nz_chunk_typeclass;
extern const struct nz_typeclass nz_string_typeclass;
extern const struct nz_typeclass nz_array_typeclass;
 
nz_rc nz_types_init(struct nz_context * context);

#endif
