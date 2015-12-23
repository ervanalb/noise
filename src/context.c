#include <stdlib.h>
#include <dlfcn.h>

#include "libnoise.h"

// TODO put these in a more reasonable spot
const size_t nz_chunk_size = 128;
const size_t nz_frame_rate = 44100;

struct nz_lib {
    void * lib_handle;
    const struct nz_typeclass const ** lib_typeclass_p_array;
    const struct nz_blockclass const ** lib_blockclass_p_array;
};

static nz_rc nz_typesystem_init(struct nz_context * context_p) {
    context_p->context_n_typeclasses = 0;
    context_p->context_typeclass_capacity = 16;
    context_p->context_typeclasses = calloc(context_p->context_typeclass_capacity, sizeof(struct nz_typeclass *));
    if(context_p->context_typeclasses == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

nz_rc nz_context_register_typeclass(struct nz_context * context_p, struct nz_typeclass const * typeclass_p) {
    if(context_p->context_n_typeclasses < context_p->context_typeclass_capacity) {
        context_p->context_typeclass_capacity *= 2;
        struct nz_typeclass const ** newptr =  realloc(context_p->context_typeclasses, context_p->context_typeclass_capacity * sizeof(struct nz_typeclass *));
        if(newptr == NULL) {
            free(context_p->context_typeclasses);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
        context_p->context_typeclasses = newptr;
    }

    context_p->context_typeclasses[context_p->context_n_typeclasses] = typeclass_p;
    context_p->context_n_typeclasses++;

    return NZ_SUCCESS;
}

static void nz_typesystem_term(struct nz_context * context_p) {
    free(context_p->context_typeclasses);
}


static nz_rc nz_blocksystem_init(struct nz_context * context_p) {
    context_p->context_n_blockclasses = 0;
    context_p->context_blockclass_capacity = 16;
    context_p->context_blockclasses = calloc(context_p->context_blockclass_capacity + 1, sizeof(struct nz_blockclass *));
    if(context_p->context_blockclasses == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

nz_rc nz_context_register_blockclass(struct nz_context * context_p, struct nz_blockclass const * blockclass_p) {
    if(context_p->context_n_blockclasses < context_p->context_blockclass_capacity) {
        context_p->context_blockclass_capacity *= 2;
        struct nz_blockclass const ** newptr =  realloc(context_p->context_blockclasses, context_p->context_blockclass_capacity * sizeof(struct nz_blockclass *));
        if(newptr == NULL) {
            free(context_p->context_blockclasses);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
        context_p->context_blockclasses = newptr;
    }

    context_p->context_blockclasses[context_p->context_n_blockclasses] = blockclass_p;
    context_p->context_n_blockclasses++;

    return NZ_SUCCESS;
}

static void nz_blocksystem_term(struct nz_context * context_p) {
    free(context_p->context_blockclasses);
}

nz_rc nz_context_create(struct nz_context ** context_pp) {
    nz_rc rc;
    *context_pp = malloc(sizeof(struct nz_context));
    if(!*context_pp) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    rc = nz_typesystem_init(*context_pp);
    if(rc != NZ_SUCCESS) {
        free(*context_pp);
        return rc;
    }
    rc = nz_blocksystem_init(*context_pp);
    if(rc != NZ_SUCCESS) {
        nz_typesystem_term(*context_pp);
        free(*context_pp);
        return rc;
    }
    return NZ_SUCCESS;
}

nz_rc nz_context_load_lib(struct nz_context * context_p, const char * lib_name, struct nz_lib ** lib) {
    *lib = calloc(sizeof(struct nz_lib), 1);
    if(*lib == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    void * handle = dlopen(lib_name, RTLD_NOW);
    if(handle == NULL) {
        free(*lib);
        NZ_RETURN_ERR_MSG(NZ_CANT_LOAD_LIBRARY, strdup(dlerror()));
    }
    const struct nz_typeclass const ** typeclass_p_array =
        (const struct nz_typeclass const **)dlsym(handle, "nz_typeclass_p_array");
    if(typeclass_p_array == NULL) {
        char * err = strdup(dlerror());
        dlclose(handle);
        free(*lib);
        NZ_RETURN_ERR_MSG(NZ_CANT_LOAD_LIBRARY, err);
    }
    const struct nz_blockclass const ** blockclass_p_array =
        (const struct nz_blockclass const **)dlsym(handle, "nz_blockclass_p_array");
    if(blockclass_p_array == NULL) {
        char * err = strdup(dlerror());
        dlclose(handle);
        free(*lib);
        NZ_RETURN_ERR_MSG(NZ_CANT_LOAD_LIBRARY, err);
    }

    nz_rc rc;

    const struct nz_typeclass ** typeclass_pp;
    for(typeclass_pp = typeclass_p_array; *typeclass_pp != NULL; typeclass_pp++) {
        rc = nz_context_register_typeclass(context_p, *typeclass_pp);
        if(rc != NZ_SUCCESS) {
            // TODO unregister previously registered types
            dlclose(handle);
            free(*lib);
            return rc;
        }
    }
    const struct nz_blockclass ** blockclass_pp;
    for(blockclass_pp = blockclass_p_array; *blockclass_pp != NULL; blockclass_pp++) {
        rc = nz_context_register_blockclass(context_p, *blockclass_pp);
        if(rc != NZ_SUCCESS) {
            // TODO unregister previously registered types & blocks
            dlclose(handle);
            free(*lib);
            return rc;
        }
    }

    (*lib)->lib_handle = handle;
    (*lib)->lib_typeclass_p_array = typeclass_p_array;
    (*lib)->lib_blockclass_p_array = blockclass_p_array;

    return NZ_SUCCESS;
}

void nz_context_unload_lib(struct nz_context * context_p, struct nz_lib * lib) {
    // TODO unregister previously registered types & blocks
    dlclose(lib->lib_handle);
    free(lib);
}

void nz_context_destroy(struct nz_context * context_pp) {
    nz_typesystem_term(context_pp);
    nz_blocksystem_term(context_pp);
    free(context_pp);
}
