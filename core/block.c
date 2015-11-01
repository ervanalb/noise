#include <string.h>
#include <stdlib.h>

#include "core/block.h"
#include "core/util.h"

// TODO move these somewhere sane
const struct nz_blockclass nz_constant_blockclass;
const struct nz_blockclass nz_debug_blockclass;

nz_rc nz_blocks_init(struct nz_context * context_p) {
    nz_rc rc;
    rc = nz_context_register_blockclass(context_p, &nz_constant_blockclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_context_register_blockclass(context_p, &nz_debug_blockclass); if(rc != NZ_SUCCESS) return rc;
    return NZ_SUCCESS;
}

// --

nz_rc nz_block_create(const struct nz_context * context, const struct nz_blockclass ** blockclass, nz_block_state ** state, struct nz_block_info * block_info, const char * string) {
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

    NZ_RETURN_ERR_MSG(NZ_BLOCK_NOT_FOUND, strdup(string));
}

nz_obj * nz_null_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    return NULL;
}

// --

void block_info_term(struct nz_block_info * info_p) {
    if(info_p->block_input_port_array != NULL) {
        for(size_t i = 0; i < info_p->block_n_inputs; i++) free(info_p->block_input_port_array[i].block_port_name);
        free(info_p->block_input_port_array);
        info_p->block_input_port_array = NULL;
    }
    if(info_p->block_output_port_array != NULL) {
        for(size_t i = 0; i < info_p->block_n_outputs; i++) free(info_p->block_output_port_array[i].block_port_name);
        free(info_p->block_output_port_array);
        info_p->block_output_port_array = NULL;
    }
    free(info_p->block_pull_fns);
    info_p->block_pull_fns = NULL;
}

nz_rc block_info_set_n_io(struct nz_block_info * info_p, size_t n_inputs, size_t n_outputs) {
    if(info_p == NULL) return NZ_SUCCESS;
    info_p->block_n_inputs = n_inputs;
    info_p->block_n_outputs = n_outputs;
    info_p->block_input_port_array = NULL;
    info_p->block_output_port_array = NULL;
    info_p->block_pull_fns = NULL;

    if(n_inputs > 0) {
        info_p->block_input_port_array = calloc(n_inputs, sizeof(struct nz_port_info));
    }
    if(n_outputs > 0) {
        info_p->block_output_port_array = calloc(n_outputs, sizeof(struct nz_port_info));
        info_p->block_pull_fns = calloc(n_outputs, sizeof(nz_pull_fn *));
    }

    if((n_inputs > 0 && info_p->block_input_port_array == NULL) ||
       (n_outputs > 0 && (info_p->block_output_port_array == NULL || info_p->block_pull_fns == NULL))) {
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }
    return NZ_SUCCESS;
}

nz_rc block_info_set_input(struct nz_block_info * info_p, size_t input_index, char * name, const struct nz_typeclass * typeclass_p, nz_type * type_p) {
    if(info_p == NULL) return NZ_SUCCESS;
    if(name == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    info_p->block_input_port_array[input_index].block_port_name = name;
    info_p->block_input_port_array[input_index].block_port_typeclass_p = typeclass_p;
    info_p->block_input_port_array[input_index].block_port_type_p = type_p;
    return NZ_SUCCESS;
}

nz_rc block_info_set_output(struct nz_block_info * info_p, size_t output_index, char * name, const struct nz_typeclass * typeclass_p, nz_type * type_p, nz_pull_fn * pull_fn_p) {
    if(info_p == NULL) return NZ_SUCCESS;
    if(name == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    info_p->block_output_port_array[output_index].block_port_name = name;
    info_p->block_output_port_array[output_index].block_port_typeclass_p = typeclass_p;
    info_p->block_output_port_array[output_index].block_port_type_p = type_p;
    info_p->block_pull_fns[output_index] = pull_fn_p;
    return NZ_SUCCESS;
}

