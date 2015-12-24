#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include "libnoise.h"

// TODO put these in a more reasonable spot
const size_t nz_chunk_size = 128;
const size_t nz_frame_rate = 44100;

struct nz_lib {
    void * lib_handle;
    const struct nz_typeclass const ** lib_typeclass_p_array;
    const struct nz_blockclass const ** lib_blockclass_p_array;
};

struct typeclass_entry;
struct typeclass_entry {
    const struct nz_typeclass * typeclass_p;
    struct typeclass_entry * next_p;
};

struct blockclass_entry;
struct blockclass_entry {
    const struct nz_blockclass * blockclass_p;
    struct blockclass_entry * next_p;
};

struct nz_context {
    struct typeclass_entry context_typeclasses_head;
    struct blockclass_entry context_blockclasses_head;
};

static nz_rc register_typeclass(struct nz_context * context_p, struct nz_typeclass const * typeclass_p) {
    struct typeclass_entry * entry_p;
    for(entry_p = &context_p->context_typeclasses_head;
        entry_p->next_p != NULL;
        entry_p = entry_p->next_p) {
        if(strcmp(entry_p->next_p->typeclass_p->type_id, typeclass_p->type_id) == 0) {
            NZ_RETURN_ERR_MSG(NZ_DUPLICATE_TYPE, strdup(typeclass_p->type_id));
        }
    }

    struct typeclass_entry * new = calloc(sizeof(struct typeclass_entry), 1);
    if(new == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    new->typeclass_p = typeclass_p;
    entry_p->next_p = new;

    return NZ_SUCCESS;
}

static void unregister_typeclass(struct nz_context * context_p, struct nz_typeclass const * typeclass_p) {
    struct typeclass_entry * entry_p;
    for(entry_p = &context_p->context_typeclasses_head;
        entry_p->next_p != NULL;
        entry_p = entry_p->next_p) {
        if(entry_p->next_p->typeclass_p == typeclass_p) {
            break;
        }
    }

    if(entry_p->next_p == NULL) return;

    struct typeclass_entry * to_delete = entry_p->next_p;
    entry_p->next_p = entry_p->next_p->next_p;
    free(to_delete);
}

static nz_rc register_blockclass(struct nz_context * context_p, struct nz_blockclass const * blockclass_p) {
    struct blockclass_entry * entry_p;
    for(entry_p = &context_p->context_blockclasses_head;
        entry_p->next_p != NULL;
        entry_p = entry_p->next_p) {
        if(strcmp(entry_p->next_p->blockclass_p->block_id, blockclass_p->block_id) == 0) {
            NZ_RETURN_ERR_MSG(NZ_DUPLICATE_BLOCK, strdup(blockclass_p->block_id));
        }
    }

    struct blockclass_entry * new = calloc(sizeof(struct blockclass_entry), 1);
    if(new == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    new->blockclass_p = blockclass_p;
    entry_p->next_p = new;

    return NZ_SUCCESS;
}

static void unregister_blockclass(struct nz_context * context_p, struct nz_blockclass const * blockclass_p) {
    struct blockclass_entry * entry_p;
    for(entry_p = &context_p->context_blockclasses_head;
        entry_p->next_p != NULL;
        entry_p = entry_p->next_p) {
        if(entry_p->next_p->blockclass_p == blockclass_p) {
            break;
        }
    }

    if(entry_p->next_p == NULL) return; // Not found

    struct blockclass_entry * to_delete = entry_p->next_p;
    entry_p->next_p = entry_p->next_p->next_p;
    free(to_delete);
}

nz_rc nz_context_create(struct nz_context ** context_pp) {
    *context_pp = calloc(sizeof(struct nz_context), 1);
    if(!*context_pp) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

nz_rc nz_context_load_lib(struct nz_context * context_p, const char * lib_name, struct nz_lib ** lib_pp) {
    struct nz_lib * lib_p = calloc(sizeof(struct nz_lib), 1);
    if(lib_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    void * handle = dlopen(lib_name, RTLD_NOW);
    if(handle == NULL) {
        free(lib_p);
        NZ_RETURN_ERR_MSG(NZ_CANT_LOAD_LIBRARY, strdup(dlerror()));
    }
    const struct nz_typeclass const ** typeclass_p_array =
        (const struct nz_typeclass const **)dlsym(handle, "nz_typeclass_p_array");
    if(typeclass_p_array == NULL) {
        char * err = strdup(dlerror());
        dlclose(handle);
        free(lib_p);
        NZ_RETURN_ERR_MSG(NZ_CANT_LOAD_LIBRARY, err);
    }
    const struct nz_blockclass const ** blockclass_p_array =
        (const struct nz_blockclass const **)dlsym(handle, "nz_blockclass_p_array");
    if(blockclass_p_array == NULL) {
        char * err = strdup(dlerror());
        dlclose(handle);
        free(lib_p);
        NZ_RETURN_ERR_MSG(NZ_CANT_LOAD_LIBRARY, err);
    }

    nz_rc rc;

    const struct nz_typeclass ** typeclass_pp;
    for(typeclass_pp = typeclass_p_array; *typeclass_pp != NULL; typeclass_pp++) {
        rc = register_typeclass(context_p, *typeclass_pp);
        if(rc != NZ_SUCCESS) {
            const struct nz_typeclass ** del_typeclass_pp;
            for(del_typeclass_pp = typeclass_p_array; del_typeclass_pp != typeclass_pp; del_typeclass_pp++) {
                unregister_typeclass(context_p, *del_typeclass_pp);
            }
            dlclose(handle);
            free(lib_p);
            return rc;
        }
    }
    const struct nz_blockclass ** blockclass_pp;
    for(blockclass_pp = blockclass_p_array; *blockclass_pp != NULL; blockclass_pp++) {
        rc = register_blockclass(context_p, *blockclass_pp);
        if(rc != NZ_SUCCESS) {
            const struct nz_typeclass ** del_typeclass_pp;
            const struct nz_blockclass ** del_blockclass_pp;
            for(del_typeclass_pp = typeclass_p_array; *del_typeclass_pp != NULL; del_typeclass_pp++) {
                unregister_typeclass(context_p, *del_typeclass_pp);
            }
            for(del_blockclass_pp = blockclass_p_array; del_blockclass_pp != blockclass_pp; del_blockclass_pp++) {
                unregister_blockclass(context_p, *del_blockclass_pp);
            }
            dlclose(handle);
            free(lib_p);
            return rc;
        }
    }

    lib_p->lib_handle = handle;
    lib_p->lib_typeclass_p_array = typeclass_p_array;
    lib_p->lib_blockclass_p_array = blockclass_p_array;
    *lib_pp = lib_p;

    return NZ_SUCCESS;
}

void nz_context_unload_lib(struct nz_context * context_p, struct nz_lib * lib_p) {
    const struct nz_typeclass ** del_typeclass_pp;
    const struct nz_blockclass ** del_blockclass_pp;
    for(del_typeclass_pp = lib_p->lib_typeclass_p_array; *del_typeclass_pp != NULL; del_typeclass_pp++) {
        unregister_typeclass(context_p, *del_typeclass_pp);
    }
    for(del_blockclass_pp = lib_p->lib_blockclass_p_array; *del_blockclass_pp != NULL; del_blockclass_pp++) {
        unregister_blockclass(context_p, *del_blockclass_pp);
    }
    dlclose(lib_p->lib_handle);
    free(lib_p);
}

void nz_context_destroy(struct nz_context * context_pp) {
    free(context_pp);
}

// --

nz_rc nz_context_create_type(const struct nz_context * context_p, const struct nz_typeclass ** typeclass_pp, nz_type ** type_pp, const char * string) {
    struct typeclass_entry * entry_p;
    for(entry_p = context_p->context_typeclasses_head.next_p;
        entry_p != NULL;
        entry_p = entry_p->next_p) {

        size_t c = 0;
        const char * type_id = entry_p->typeclass_p->type_id;
        size_t len = strlen(string);
        for(;;) {
            if(type_id[c] == '\0') {
                if(string[c] == '\0') {
                    // Match, no args
                    *typeclass_pp = entry_p->typeclass_p;
                    return (*typeclass_pp)->type_create(context_p, type_pp, NULL);
                } else if(string[c] == '<' && string[len - 1] == '>') {
                    // Match, args
                    char * args = strndup(&(string[c + 1]), len - 2 - c);
                    if(args == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                    *typeclass_pp = entry_p->typeclass_p;
                    nz_rc rc = (*typeclass_pp)->type_create(context_p, type_pp, args);
                    free(args);
                    return rc;
                } else {
                    break; // No match, type ended early
                }
            } else if(string[c] == '\0') {
                break; // No match, string ended early
            } else if(string[c] == type_id[c]) {
                c++;
            } else {
                break;
            }
        }
    }

    NZ_RETURN_ERR(NZ_TYPE_NOT_FOUND);
}

nz_rc nz_context_list_types(struct nz_context * context_p, char const *** typeclass_string_array_p) {
    struct typeclass_entry * entry_p;
    size_t n = 0;
    for(entry_p = context_p->context_typeclasses_head.next_p;
        entry_p != NULL;
        entry_p = entry_p->next_p) {
        n++;
    }

    char const ** typeclass_string_array = calloc(n + 1, sizeof(const char *));
    if(typeclass_string_array == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    n = 0;
    for(entry_p = context_p->context_typeclasses_head.next_p;
        entry_p != NULL;
        entry_p = entry_p->next_p) {
        typeclass_string_array[n++] = entry_p->typeclass_p->type_id;
    }

    *typeclass_string_array_p = typeclass_string_array;
    return NZ_SUCCESS;
}

void nz_context_free_type_list(char const ** typeclass_string_array) {
    free(typeclass_string_array);
}

// --

nz_rc nz_context_create_block(const struct nz_context * context_p, const struct nz_blockclass ** blockclass, nz_block_state ** state, struct nz_block_info * block_info, const char * string) {
    // Create a block from a spec `string`
    // `struct nz_context * context_p`: input nz_context, which contains all registered blockclasses
    // `char * string`: input like "tee(2)" or "accumulator"
    // `struct nz_blockclass ** blockclass`: output blockclass for newly created block
    // `nz_block_state ** state`: output block state for newly created block
    // `struct nz_block_info * block_info`: output block info for newly created block, already allocated

    struct blockclass_entry * entry_p;
    for(entry_p = context_p->context_blockclasses_head.next_p;
        entry_p != NULL;
        entry_p = entry_p->next_p) {

        size_t c = 0;
        const char * block_id = entry_p->blockclass_p->block_id;
        size_t len = strlen(string);

        for(;;) {
            if(block_id[c] == '\0') {
                if(string[c] == '\0') {
                    // Match, no args
                    *blockclass = entry_p->blockclass_p;
                    return (*blockclass)->block_create(context_p, NULL, state, block_info);
                } else if(string[c] == '(' && string[len - 1] == ')') {
                    // Match, args
                    char * args = strndup(&(string[c + 1]), len - 2 - c);
                    if(args == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                    *blockclass = entry_p->blockclass_p;
                    nz_rc rc = (*blockclass)->block_create(context_p, args, state, block_info);
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

nz_rc nz_context_list_blocks(struct nz_context * context_p, char const *** blockclass_string_array_p) {
    struct blockclass_entry * entry_p;
    size_t n = 0;
    for(entry_p = context_p->context_blockclasses_head.next_p;
        entry_p != NULL;
        entry_p = entry_p->next_p) {
        n++;
    }

    char const ** blockclass_string_array = calloc(n + 1, sizeof(const char *));
    if(blockclass_string_array == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    n = 0;
    for(entry_p = context_p->context_blockclasses_head.next_p;
        entry_p != NULL;
        entry_p = entry_p->next_p) {
        blockclass_string_array[n++] = entry_p->blockclass_p->block_id;
    }

    *blockclass_string_array_p = blockclass_string_array;
    return NZ_SUCCESS;
}

void nz_context_free_block_list(char const ** blockclass_string_array) {
    free(blockclass_string_array);
}


