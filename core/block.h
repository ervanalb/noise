#ifndef __CORE_BLOCK_H__
#define __CORE_BLOCK_H__

#include <stddef.h>

#include "core/error.h"
#include "core/util.h"
#include "core/ntype.h"
#include "core/context.h"

struct nz_block;

typedef void nz_block_state;
typedef nz_obj * nz_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p);

struct nz_block {
    nz_block_state *  block_state_p;
    nz_pull_fn **     block_upstream_pull_fn_p_array;
    struct nz_block * block_upstream_block_array;
};

struct nz_block_info {
    size_t                        block_n_inputs;
    char **                       block_input_names;
    const struct nz_typeclass **  block_input_typeclasses;
    nz_type **                    block_input_types;

    size_t                        block_n_outputs;
    char **                       block_output_names;
    const struct nz_typeclass **  block_output_typeclasses;
    nz_type **                    block_output_types;
    const nz_pull_fn **           block_pull_fns;
};

struct nz_blockclass {
    const char * block_id;
    nz_rc (*block_create) (const struct nz_context * context, const char * string, nz_block_state ** state, struct nz_block_info * info);
    void (*block_destroy) (nz_block_state * state, struct nz_block_info * info);
};

// --
// Needed by context

nz_rc nz_blocks_init(struct nz_context * context);

// --

nz_rc nz_block_create(const struct nz_context * context, const char * string, const struct nz_blockclass ** blockclass_pp, nz_block_state ** state_pp, struct nz_block_info * info_p);

nz_obj * nz_null_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p);

#endif
