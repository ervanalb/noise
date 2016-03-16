#include <stdlib.h>
#include "std.h"

struct tee_block_state {
    size_t n_outputs;
    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;
    nz_obj * obj_p;
    nz_obj * result_p;
};

static nz_obj * tee_main_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct tee_block_state * tee_block_state_p = (struct tee_block_state *)(self.block_state_p);

    tee_block_state_p->result_p = NZ_PULL(self, 0, tee_block_state_p->obj_p);

    if(tee_block_state_p->result_p == NULL) return NULL;

    if(tee_block_state_p->typeclass_p->type_copy_obj(tee_block_state_p->type_p, obj_p, tee_block_state_p->obj_p) != NZ_SUCCESS) {
        // THROW PULL-TIME ERROR
        return NULL;
    }

    return obj_p;
}

static nz_obj * tee_aux_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct tee_block_state * tee_block_state_p = (struct tee_block_state *)(self.block_state_p);

    if(tee_block_state_p->result_p == NULL) return NULL;

    if(tee_block_state_p->typeclass_p->type_copy_obj(tee_block_state_p->type_p, obj_p, tee_block_state_p->obj_p) != NZ_SUCCESS) {
        // THROW PULL-TIME ERROR
        return NULL;
    }

    return obj_p;
}

static nz_rc tee_block_create_args(size_t n_outputs, const struct nz_typeclass * typeclass_p, nz_type * type_p, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct tee_block_state * state_p = calloc(1, sizeof(struct tee_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    if(n_outputs < 1) NZ_RETURN_ERR(NZ_ARG_VALUE);

    // This function assumes ownership of typeclass_p and type_p
    nz_rc rc;
    nz_obj * obj_p;
    rc = typeclass_p->type_create_obj(type_p, &obj_p);
    if(rc != NZ_SUCCESS) {
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }

    state_p->typeclass_p = typeclass_p;
    state_p->type_p = type_p;
    state_p->obj_p = obj_p;

    if((rc = nz_block_info_set_n_io(info_p, 1, n_outputs)) == NZ_SUCCESS &&
       (rc = nz_block_info_set_input(info_p, 0, strdup("in"), typeclass_p, type_p)) == NZ_SUCCESS &&
       (rc = nz_block_info_set_output(info_p, 0, strdup("main"), typeclass_p, type_p, tee_main_pull_fn)) == NZ_SUCCESS) {
        for(size_t i = 1; i < n_outputs; i++) {
            if((rc = nz_block_info_set_output(info_p, i, rsprintf("aux %lu", i), typeclass_p, type_p, tee_aux_pull_fn)) != NZ_SUCCESS) break;
        }
    }

    if(rc != NZ_SUCCESS) {
        typeclass_p->type_destroy_obj(type_p, obj_p);
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }
 
    *(struct tee_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc tee_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[2];
    nz_rc rc = arg_parse("required int n_outputs, required generic type", string, args);
    if(rc != NZ_SUCCESS) return rc;
    long n_outputs = *(long *)args[0];
    char * type_str = (char *)args[1];
    free(args[0]);

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    if(n_outputs < 0) {
        free(type_str);
        NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, rsprintf("%ld", n_outputs));
    }

    rc = nz_context_create_type(context_p, &typeclass_p, &type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    return tee_block_create_args(n_outputs, typeclass_p, type_p, state_pp, info_p);
}

void tee_block_destroy(nz_block_state * state_p) {
    struct tee_block_state * tee_block_state_p = (struct tee_block_state *)state_p;
    tee_block_state_p->typeclass_p->type_destroy_obj(tee_block_state_p->type_p, tee_block_state_p->obj_p);
    tee_block_state_p->typeclass_p->type_destroy(tee_block_state_p->type_p);
    free(state_p);
}

NZ_DECLARE_BLOCKCLASS(tee)

struct wye_block_state {
    size_t n_inputs;
    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;
    nz_obj * obj_p;
    nz_obj * result_p;
};

nz_obj * wye_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct wye_block_state * wye_block_state_p = (struct wye_block_state *)(self.block_state_p);

    for(size_t i = 0; i < wye_block_state_p->n_inputs; i++) {
        wye_block_state_p->result_p = NZ_PULL(self, 0, wye_block_state_p->obj_p);
    }

    if(wye_block_state_p->result_p == NULL) return NULL;

    if(wye_block_state_p->typeclass_p->type_copy_obj(wye_block_state_p->type_p, obj_p, wye_block_state_p->obj_p) != NZ_SUCCESS) {
        // THROW PULL-TIME ERROR
        return NULL;
    }

    return obj_p;
}

static nz_rc wye_block_create_args(size_t n_inputs, const struct nz_typeclass * typeclass_p, nz_type * type_p, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct wye_block_state * state_p = calloc(1, sizeof(struct wye_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    if(n_inputs < 1) NZ_RETURN_ERR(NZ_ARG_VALUE);

    // This function assumes ownership of typeclass_p and type_p
    nz_rc rc;
    nz_obj * obj_p;
    rc = typeclass_p->type_create_obj(type_p, &obj_p);
    if(rc != NZ_SUCCESS) {
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }

    state_p->typeclass_p = typeclass_p;
    state_p->type_p = type_p;
    state_p->obj_p = obj_p;

    if((rc = nz_block_info_set_n_io(info_p, n_inputs, 1)) == NZ_SUCCESS &&
       (rc = nz_block_info_set_output(info_p, 0, strdup("out"), typeclass_p, type_p, wye_pull_fn)) == NZ_SUCCESS &&
       (rc = nz_block_info_set_input(info_p, n_inputs - 1, strdup("main"), typeclass_p, type_p)) == NZ_SUCCESS) {
        for(size_t i = 0; i < n_inputs - 1; i++) {
            if((rc = nz_block_info_set_input(info_p, i, rsprintf("aux %lu", i + 1), typeclass_p, type_p)) != NZ_SUCCESS) break;
        }
    }

    if(rc != NZ_SUCCESS) {
        typeclass_p->type_destroy_obj(type_p, obj_p);
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }
 
    *(struct wye_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc wye_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[2];
    nz_rc rc = arg_parse("required int n_inputs, required generic type", string, args);
    if(rc != NZ_SUCCESS) return rc;
    long n_inputs = *(long *)args[0];
    char * type_str = (char *)args[1];
    free(args[0]);

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    if(n_inputs < 0) {
        free(type_str);
        NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, rsprintf("%ld", n_inputs));
    }

    rc = nz_context_create_type(context_p, &typeclass_p, &type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    return wye_block_create_args(n_inputs, typeclass_p, type_p, state_pp, info_p);
}

void wye_block_destroy(nz_block_state * state_p) {
    struct wye_block_state * wye_block_state_p = (struct wye_block_state *)state_p;
    wye_block_state_p->typeclass_p->type_destroy_obj(wye_block_state_p->type_p, wye_block_state_p->obj_p);
    wye_block_state_p->typeclass_p->type_destroy(wye_block_state_p->type_p);
    free(state_p);
}

NZ_DECLARE_BLOCKCLASS(wye)

struct any_block_state {
    size_t n_inputs;
    size_t last_idx;
    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;
    nz_obj * obj_p;
};

nz_obj * any_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct any_block_state * any_block_state_p = (struct any_block_state *)(self.block_state_p);
    nz_obj * result_p;
    
    size_t first_idx = any_block_state_p->last_idx;
    bool found = false;
    bool copy_error = false;

    for(size_t _i = 0; _i < any_block_state_p->n_inputs; _i++) {
        // Start with whichever block worked last time
        size_t i = (_i + first_idx) % any_block_state_p->n_inputs;

        result_p = NZ_PULL(self, 0, any_block_state_p->obj_p);
        if (!found && result_p != NULL) {
            found = true;
            any_block_state_p->last_idx = i;
            if(any_block_state_p->typeclass_p->type_copy_obj(any_block_state_p->type_p, obj_p, any_block_state_p->obj_p) != NZ_SUCCESS)
                copy_error = true;
        }
    }

    if (copy_error || !found)
        return NULL;
    else
        return obj_p;
}

static nz_rc any_block_create_args(size_t n_inputs, const struct nz_typeclass * typeclass_p, nz_type * type_p, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct any_block_state * state_p = calloc(1, sizeof(struct any_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    if(n_inputs < 1) NZ_RETURN_ERR(NZ_ARG_VALUE);

    // This function assumes ownership of typeclass_p and type_p
    nz_rc rc;
    nz_obj * obj_p;
    rc = typeclass_p->type_create_obj(type_p, &obj_p);
    if(rc != NZ_SUCCESS) {
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }

    state_p->typeclass_p = typeclass_p;
    state_p->type_p = type_p;
    state_p->obj_p = obj_p;
    state_p->last_idx = 0;

    if((rc = nz_block_info_set_n_io(info_p, n_inputs, 1)) == NZ_SUCCESS &&
       (rc = nz_block_info_set_output(info_p, 0, strdup("out"), typeclass_p, type_p, any_pull_fn)) == NZ_SUCCESS) {
        for(size_t i = 0; i < n_inputs - 1; i++) {
            if((rc = nz_block_info_set_input(info_p, i, rsprintf("in %lu", i + 1), typeclass_p, type_p)) != NZ_SUCCESS) break;
        }
    }

    if(rc != NZ_SUCCESS) {
        typeclass_p->type_destroy_obj(type_p, obj_p);
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }
 
    *(struct any_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc any_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[2];
    nz_rc rc = arg_parse("required int n_inputs, required generic type", string, args);
    if(rc != NZ_SUCCESS) return rc;
    long n_inputs = *(long *)args[0];
    char * type_str = (char *)args[1];
    free(args[0]);

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    if(n_inputs < 0) {
        free(type_str);
        NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, rsprintf("%ld", n_inputs));
    }

    rc = nz_context_create_type(context_p, &typeclass_p, &type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    return any_block_create_args(n_inputs, typeclass_p, type_p, state_pp, info_p);
}

void any_block_destroy(nz_block_state * state_p) {
    struct any_block_state * any_block_state_p = (struct any_block_state *)state_p;
    any_block_state_p->typeclass_p->type_destroy_obj(any_block_state_p->type_p, any_block_state_p->obj_p);
    any_block_state_p->typeclass_p->type_destroy(any_block_state_p->type_p);
    free(state_p);
}

NZ_DECLARE_BLOCKCLASS(any)

struct gate_block_state {
    size_t n_inputs;
    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;
    nz_obj * obj_p;
};

nz_obj * gate_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct gate_block_state * gate_block_state_p = (struct gate_block_state *)(self.block_state_p);
    nz_obj * result_p;

    nz_real gateval = 0;
    
    if(NZ_PULL(self, 0, &gateval) == NULL)
        return NULL;

    return NZ_PULL(self, 1, obj_p);
}

static nz_rc gate_block_create_args(const struct nz_typeclass * typeclass_p, nz_type * type_p, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct gate_block_state * state_p = calloc(1, sizeof(struct gate_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    // This function assumes ownership of typeclass_p and type_p
    nz_rc rc;
    nz_obj * obj_p;
    rc = typeclass_p->type_create_obj(type_p, &obj_p);
    if(rc != NZ_SUCCESS) {
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }

    state_p->typeclass_p = typeclass_p;
    state_p->type_p = type_p;
    state_p->obj_p = obj_p;

    if((rc = nz_block_info_set_n_io(info_p, 2, 1)) != NZ_SUCCESS) goto fail;
    if((rc = nz_block_info_set_output(info_p, 0, strdup("out"), typeclass_p, type_p, gate_pull_fn)) != NZ_SUCCESS) goto fail;
    if((rc = nz_block_info_set_input(info_p, 0, strdup("gate"), typeclass_p, type_p)) != NZ_SUCCESS) goto fail;
    if((rc = nz_block_info_set_input(info_p, 1, strdup("in"), &nz_real_typeclass, NULL)) != NZ_SUCCESS) goto fail;

    *(struct gate_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;

fail:
    typeclass_p->type_destroy_obj(type_p, obj_p);
    typeclass_p->type_destroy(type_p);
    free(state_p);
    return rc;
}

nz_rc gate_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required generic type", string, args);
    if(rc != NZ_SUCCESS) return rc;
    char * type_str = (char *)args[0];

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    rc = nz_context_create_type(context_p, &typeclass_p, &type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    return gate_block_create_args(typeclass_p, type_p, state_pp, info_p);
}

void gate_block_destroy(nz_block_state * state_p) {
    struct gate_block_state * gate_block_state_p = (struct gate_block_state *)state_p;
    gate_block_state_p->typeclass_p->type_destroy_obj(gate_block_state_p->type_p, gate_block_state_p->obj_p);
    gate_block_state_p->typeclass_p->type_destroy(gate_block_state_p->type_p);
    free(state_p);
}

NZ_DECLARE_BLOCKCLASS(gate)
