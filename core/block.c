#include <string.h>
#include <stdlib.h>

#include "core/block.h"
#include "core/util.h"

static void free_block_info(struct nz_block_info * info) {
    // Destroy types
    for(size_t i = 0; i < info->block_n_inputs; i++) {
        free(info->block_input_names[i]);
        nz_type_destroy(&info->block_input_typeclasses[i], &info->block_input_types[i]);
    }

    for(size_t i = 0; i < info->block_n_outputs; i++) {
        free(info->block_output_names[i]);
        nz_type_destroy(&info->block_output_typeclasses[i], &info->block_output_types[i]);
    }

    // Free strings and arrays
    free(info->block_input_names);
    free(info->block_input_typeclasses);
    free(info->block_input_types);

    free(info->block_output_names);
    free(info->block_output_typeclasses);
    free(info->block_output_types);

    free(info->block_pull_fns);
}

// --

nz_rc nz_blocks_init(struct nz_context * context) {
    // XXX Is this actually a nop? -zbanks
    // What is the intention of this?
    return NZ_SUCCESS;
}

// --

nz_rc nz_block_create(struct nz_context * context, const char * string, const struct nz_blockclass ** blockclass, nz_block_state ** state, struct nz_block_info * block_info) {
    // Create a block from a spec `string`
    // `struct nz_context * context`: input nz_context, which contains all registered blockclasses
    // `char * string`: input like "tee(2)" or "accumulator"
    // `struct nz_blockclass ** blockclass`: output blockclass for newly created block
    // `nz_block_state ** state`: output block state for newly created block
    // `struct nz_block_info * block_info`: output block info for newly created block, already allocated
    for(size_t i = 0; i < context->context_n_blockclasses; i++) {
        size_t c = 0;
        const char * block_id = context->context_blockclasses[i]->block_id;
        size_t len = strlen(string);
        for(;;) {
            if(block_id[c] == '\0') {
                if(string[c] == '\0') {
                    // Match, no args
                    *blockclass= context->context_blockclasses[i];
                    return (*blockclass)->block_create(context, NULL, state, block_info);
                } else if(string[c] == '(' && string[len - 1] == ')') {
                    // Match, args
                    char * args = strndup(&(string[c + 1]), len - 2 - c);
                    if(args == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                    *blockclass= context->context_blockclasses[i];
                    nz_rc rc = (*blockclass)->block_create(context, args, state, block_info);
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

void nz_block_destroy(const struct nz_blockclass ** blockclass, nz_block_state ** state, struct nz_block_info * info) {
    (*blockclass)->block_destroy(state);
    free_block_info(info);    
}
