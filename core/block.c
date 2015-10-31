#include <string.h>
#include <stdlib.h>

#include "core/block.h"
#include "core/util.h"

static void free_block_info(struct nz_block_info * info_p) {
    // Destroy types
    for(size_t i = 0; i < info_p->block_n_inputs; i++) {
        free(info_p->block_input_name_array[i]);
        nz_type_destroy(&info_p->block_input_typeclass_array[i], &info_p->block_input_type_p_array[i]);
    }

    for(size_t i = 0; i < info_p->block_n_outputs; i++) {
        free(info_p->block_output_name_array[i]);
        nz_type_destroy(&info_p->block_output_typeclass_array[i], &info_p->block_output_type_p_array[i]);
    }

    // Free strings and arrays
    free(info_p->block_input_name_array);
    free(info_p->block_input_typeclass_array);
    free(info_p->block_input_type_p_array);

    free(info_p->block_output_typeclass_array);
    free(info_p->block_output_type_p_array);
    free(info_p->block_output_name_array);

    free(info_p->block_pull_fn_p_array);
}

// --

nz_rc nz_blocks_init(struct nz_context * context_p) {
    // XXX Is this actually a nop? -zbanks
    // What is the intention of this?
    return NZ_SUCCESS;
}

// --

nz_rc nz_block_create(struct nz_context * context_p, const char * string, const struct nz_blockclass ** blockclass_pp, nz_block_state ** state_pp, struct nz_block_info * block_info_p) {
    // Create a block from a spec `string`
    // `struct nz_context * context_p`: input nz_context, which contains all registered blockclasses
    // `char * string`: input like "tee(2)" or "accumulator"
    // `struct nz_blockclass ** blockclass_pp`: output blockclass for newly created block
    // `nz_block_state ** state_pp`: output block state for newly created block
    // `struct nz_block_info * block_info_p`: output block info for newly created block, already allocated
    for(size_t i = 0; i < context_p->context_n_blockclasses; i++) {
        size_t c = 0;
        const char * block_id = context_p->context_blockclasses[i]->block_id;
        size_t len = strlen(string);
        for(;;) {
            if(block_id[c] == '\0') {
                if(string[c] == '\0') {
                    // Match, no args
                    *blockclass_pp = context_p->context_blockclasses[i];
                    return (*blockclass_pp)->block_create(context_p, NULL, state_pp, block_info_p);
                } else if(string[c] == '(' && string[len - 1] == ')') {
                    // Match, args
                    char * args = strndup(&(string[c + 1]), len - 2 - c);
                    if(args == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                    *blockclass_pp = context_p->context_blockclasses[i];
                    nz_rc rc = (*blockclass_pp)->block_create(context_p, args, state_pp, block_info_p);
                    free(args);
                    return rc;
                } else {
                    break; // No match, block ended early
                }
            } else if(string[c] == '\0') {
                break; // No match, string ended early
            } else if(string[c] == block_id[c]) {
                c++;
            } else {
                break;
            }
        }
    }

    NZ_RETURN_ERR(NZ_BLOCK_NOT_FOUND);
}

void nz_block_destroy(const struct nz_blockclass ** blockclass_pp, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    (*blockclass_pp)->block_destroy(state_pp);
    free_block_info(info_p);    
}
