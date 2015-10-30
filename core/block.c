#include <string.h>
#include <stdlib.h>

#include "core/block.h"
#include "core/util.h"

void free_block_info(struct nz_block_info * info_p) {
    // Destroy types
    for(size_t i = 0; i < info_p->block_n_inputs; i++) {
        info_p->block_input_typeclass_p_array[i]->type_destroy(info_p->block_input_type_p_array[i]);
    }

    for(size_t i = 0; i < info_p->block_n_outputs; i++) {
        info_p->block_output_typeclass_p_array[i]->type_destroy(info_p->block_output_type_p_array[i]);
    }

    // Free strings and arrays
    free(info_p->block_input_name_array);
    free(info_p->block_output_name_array);
    free(info_p->block_input_typeclass_p_array);
    free(info_p->block_input_type_p_array);
    free(info_p->block_output_typeclass_p_array);
    free(info_p->block_output_type_p_array);
    free(info_p->block_pull_fn_p_array);
}

// --

nz_rc nz_init_block_system(struct nz_context * context_p) {
    context_p->n_registered_blockclasses = 0;
    context_p->registered_blockclass_capacity = 16;
    context_p->registered_blockclasses = calloc(context_p->registered_blockclass_capacity, sizeof(struct nz_blockclass *));
    if(context_p->registered_blockclasses == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

nz_rc nz_register_blockclass(struct nz_context * context_p, struct nz_blockclass const * blockclass_p) {
    if(context_p->n_registered_blockclasses < context_p->registered_blockclass_capacity) {
        context_p->registered_blockclass_capacity *= 2;
        struct nz_blockclass const ** newptr =  realloc(context_p->registered_blockclasses, context_p->registered_blockclass_capacity * sizeof(struct nz_blockclass *));
        if(newptr == NULL) {
            free(context_p->registered_blockclasses);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
        context_p->registered_blockclasses = newptr;
    }

    context_p->registered_blockclasses[context_p->n_registered_blockclasses] = blockclass_p;
    context_p->n_registered_blockclasses++;

    return NZ_SUCCESS;
}

void nz_deinit_block_system(struct nz_context * context_p) {
    free(context_p->registered_blockclasses);
}

nz_rc nz_init_blocks(struct nz_context * context_p) {
    return NZ_SUCCESS;
}

// --

nz_rc nz_block_create(struct nz_context * context_p, const struct nz_blockclass ** blockclass_pp, nz_block_state ** state_pp, struct nz_block_info * block_info_p, const char * string) {
    for(size_t i = 0; i < context_p->n_registered_blockclasses; i++) {
        size_t c = 0;
        const char * block_id = context_p->registered_blockclasses[i]->block_id;
        size_t len = strlen(string);
        for(;;) {
            if(block_id[c] == '\0') {
                if(string[c] == '\0') {
                    // Match, no args
                    *blockclass_pp = context_p->registered_blockclasses[i];
                    return (*blockclass_pp)->block_create(context_p, state_pp, block_info_p, NULL);
                } else if(string[c] == '(' && string[len - 1] == ')') {
                    // Match, args
                    char * args = strndup(&(string[c + 1]), len - 2 - c);
                    if(args == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                    *blockclass_pp = context_p->registered_blockclasses[i];
                    nz_rc rc = (*blockclass_pp)->block_create(context_p, state_pp, block_info_p, args);
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
