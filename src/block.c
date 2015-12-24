#include <string.h>
#include <stdlib.h>

#include "libnoise.h"

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

