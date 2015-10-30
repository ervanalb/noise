#include "core/context.h"
#include "core/error.h"
#include "core/ntype.h"
#include "core/block.h"
#include "core/util.h"

nz_rc nz_create_context(struct nz_context ** context_pp) {
    nz_rc rc;
    *context_pp = malloc(sizeof(struct nz_context));
    if(!*context_pp) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    rc = nz_init_type_system(*context_pp);
    if(rc != NZ_SUCCESS) {
        free(*context_pp);
        return rc;
    }
    rc = nz_init_types(*context_pp);
    if(rc != NZ_SUCCESS) {
        nz_deinit_type_system(*context_pp);
        free(*context_pp);
        return rc;
    }
    rc = nz_init_block_system(*context_pp);
    if(rc != NZ_SUCCESS) {
        nz_deinit_type_system(*context_pp);
        free(*context_pp);
        return rc;
    }
    rc = nz_init_blocks(*context_pp);
    if(rc != NZ_SUCCESS) {
        nz_deinit_type_system(*context_pp);
        nz_deinit_block_system(*context_pp);
        free(*context_pp);
        return rc;
    }
     return NZ_SUCCESS;
}

void nz_destroy_context(struct nz_context * context_p) {
    nz_deinit_type_system(context_p);
    nz_deinit_block_system(context_p);
    free(context_p);
}
