#ifndef __CORE_BLOCK_H__
#define __CORE_BLOCK_H__

#include <stddef.h>

#include "core/error.h"
#include "core/util.h"
#include "core/ntype.h"
#include "core/context.h"

struct nz_block;

typedef void nz_block_state;
typedef nz_obj * pull_fn(struct nz_block self);

struct nz_block {
    nz_block_state *    block_state_p;
    const pull_fn **    block_upstream_pull_fn_p_array;
    struct nz_block *   block_upstream_p_array;
};

struct nz_block_info {
    size_t                  block_n_inputs;
    char **                 block_input_name_array;
    struct nz_typeclass *   block_input_typeclass_array;
    nz_type **              block_input_type_p_array;

    size_t                  block_n_outputs;
    char **                 block_output_name_array;
    struct nz_typeclass *   block_output_typeclass_array;
    nz_type **              block_output_type_p_array;
    const pull_fn **        block_pull_fn_p_array;
};

struct nz_blockclass {
    // Static members
    const char * block_id;
    nz_rc (*block_create)         (struct nz_context * context, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p);

    // Instance methods
    void   (*block_destroy)       (nz_block_state ** state_pp);
};

//void free_block_info(struct nz_block_info * info_p);

// --
// Needed by context

nz_rc nz_blocks_init(struct nz_context * context_p);

// --

nz_rc nz_block_create(struct nz_context * context_p, const char * string, const struct nz_blockclass ** blockclass_pp, nz_block_state ** state_pp, struct nz_block_info * info_p);
void nz_block_destroy(const struct nz_blockclass ** blockclass_pp, nz_block_state ** state_pp, struct nz_block_info * info_p);

#endif
