#include <stdlib.h>
#include "noise.h"
#include "core/ntype.h"

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

    if((rc = block_info_set_n_io(info_p, 1, n_outputs)) == NZ_SUCCESS &&
       (rc = block_info_set_input(info_p, 0, strdup("in"), typeclass_p, type_p)) == NZ_SUCCESS &&
       (rc = block_info_set_output(info_p, 0, strdup("main"), typeclass_p, type_p, tee_main_pull_fn)) == NZ_SUCCESS) {
        for(size_t i = 1; i < n_outputs; i++) {
            if((rc = block_info_set_output(info_p, i, rsprintf("aux %lu", i), typeclass_p, type_p, tee_aux_pull_fn)) != NZ_SUCCESS) break;
        }
    }

    if(rc != NZ_SUCCESS) {
        block_info_term(info_p);
        typeclass_p->type_destroy_obj(type_p, obj_p);
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }
 
    *(struct tee_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc tee_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    if(string == NULL) NZ_RETURN_ERR(NZ_EXPECTED_ARGS);

    const char * pos = string;
    const char * start;
    size_t length;
    int end;
    nz_rc rc;

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    rc = nz_next_block_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * n_outputs_str = strndup(start, length);
    if(n_outputs_str == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    size_t n_outputs;
    if(sscanf(n_outputs_str, "%lu%n", &n_outputs, &end) != 1 || end <= 0 || (size_t)end != length) {
        free(n_outputs_str);
        NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string));
    }
    free(n_outputs_str);

    rc = nz_next_block_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * type_str = strndup(start, length);
    rc = nz_type_create(context_p, &typeclass_p, &type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    if(pos != NULL) {
        typeclass_p->type_destroy(type_p);
        NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string));
    }

    return tee_block_create_args(n_outputs, typeclass_p, type_p, state_pp, info_p);
}

void tee_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct tee_block_state * tee_block_state_p = (struct tee_block_state *)state_p;
    block_info_term(info_p);
    tee_block_state_p->typeclass_p->type_destroy_obj(tee_block_state_p->type_p, tee_block_state_p->obj_p);
    tee_block_state_p->typeclass_p->type_destroy(tee_block_state_p->type_p);
    free(state_p);
}

DECLARE_BLOCKCLASS(tee)

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

    if((rc = block_info_set_n_io(info_p, n_inputs, 1)) == NZ_SUCCESS &&
       (rc = block_info_set_output(info_p, 1, strdup("out"), typeclass_p, type_p, wye_pull_fn)) == NZ_SUCCESS &&
       (rc = block_info_set_input(info_p, n_inputs - 1, strdup("main"), typeclass_p, type_p)) == NZ_SUCCESS) {
        for(size_t i = 0; i < n_inputs - 1; i++) {
            if((rc = block_info_set_input(info_p, i, rsprintf("aux %lu", i + 1), typeclass_p, type_p)) != NZ_SUCCESS) break;
        }
    }

    if(rc != NZ_SUCCESS) {
        block_info_term(info_p);
        typeclass_p->type_destroy_obj(type_p, obj_p);
        typeclass_p->type_destroy(type_p);
        free(state_p);
        return rc;
    }
 
    *(struct wye_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc wye_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    if(string == NULL) NZ_RETURN_ERR(NZ_EXPECTED_ARGS);

    const char * pos = string;
    const char * start;
    size_t length;
    int end;
    nz_rc rc;

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    rc = nz_next_block_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * n_inputs_str = strndup(start, length);
    if(n_inputs_str == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    size_t n_inputs;
    if(sscanf(n_inputs_str, "%lu%n", &n_inputs, &end) != 1 || end <= 0 || (size_t)end != length) {
        free(n_inputs_str);
        NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string));
    }
    free(n_inputs_str);

    rc = nz_next_block_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * type_str = strndup(start, length);
    rc = nz_type_create(context_p, &typeclass_p, &type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    if(pos != NULL) {
        typeclass_p->type_destroy(type_p);
        NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string));
    }

    return wye_block_create_args(n_inputs, typeclass_p, type_p, state_pp, info_p);
}

void wye_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct wye_block_state * wye_block_state_p = (struct wye_block_state *)state_p;
    block_info_term(info_p);
    wye_block_state_p->typeclass_p->type_destroy_obj(wye_block_state_p->type_p, wye_block_state_p->obj_p);
    wye_block_state_p->typeclass_p->type_destroy(wye_block_state_p->type_p);
    free(state_p);
}

DECLARE_BLOCKCLASS(wye)
