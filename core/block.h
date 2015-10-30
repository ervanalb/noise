#ifndef __CORE_BLOCK_H__
#define __CORE_BLOCK_H__

#include <stddef.h>

#include "noise.h"
#include "core/ntype.h"
#include "core/util.h"

struct nz_block;

typedef void nz_block_state;
typedef nz_obj * pull_fn(struct nz_block self);

struct nz_block {
    nz_block_state *   block_state_p;
    const pull_fn **   block_upstream_pull_fn_p_array;
    struct nz_block ** block_upstream_p_array;
};

struct nz_block_info {
    int                    n_inputs;
    int                    n_outputs;
    const char **          block_input_name_array;
    const char **          block_output_name_array;
    struct nz_typeclass ** block_input_typeclass_p_array;
    nz_type **             block_input_type_p_array;
    struct nz_typeclass ** block_output_typeclass_p_array;
    nz_type **             block_output_type_p_array;
};

struct nz_blockclass {
    // Static members
    const char * block_id;
    nz_rc (*block_create)         (nz_block_state ** state_pp, struct nz_block_info * info_p, const char * string);

    // Instance methods
    void   (*block_destroy)       (nz_block_state * state_p);
    nz_rc  (*block_to_str)        (nz_block_state * state_p, char ** string);
    nz_rc  (*block_from_str)      (nz_block_state * state_p, const char * string);
};

void free_block_info(struct nz_block_info * info_p);

nz_rc nz_init_block_system();
nz_rc nz_register_blockclass(struct nz_blockclass const * blockclass_p);
void nz_deinit_block_system();

nz_rc nz_block_create(const struct nz_blockclass ** blockclass_pp, nz_block_state ** state_pp, const char * string);

nz_rc nz_init_blocks();

#endif
