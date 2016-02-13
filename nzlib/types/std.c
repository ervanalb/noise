#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "std.h"

// --
// Declarations

struct nz_array_type {
    size_t size;
    const struct nz_typeclass * typeclass_p;
    const nz_type * type_p;
};

nz_rc array_type_create_args(nz_type ** type_pp, size_t size, const struct nz_typeclass * typeclass_p, const nz_type * type_p);

// --
// C Primitives

NZ_DECLARE_PRIMITIVE_TYPECLASS(int, nz_int, "%ld")
NZ_DECLARE_PRIMITIVE_TYPECLASS(long, nz_long, "%lld")

#ifdef NZ_REAL_FLOAT
NZ_DECLARE_PRIMITIVE_TYPECLASS(real, nz_real, "%f")
#else
NZ_DECLARE_PRIMITIVE_TYPECLASS(real, nz_real, "%lf")
#endif

// --
// Simple noise types

// Chunk
NZ_DECLARE_TYPE_ID(chunk)
NZ_GEN_SIMPLE_TYPE_FNS(chunk)
NZ_GEN_STATIC_OBJ_FNS(chunk, (sizeof(double) * nz_chunk_size)) // TODO: malloc might result in unaligned??
NZ_GEN_SHALLOW_COPY_FN(chunk, (sizeof(double) * nz_chunk_size))

static nz_rc chunk_type_init_obj(const nz_type * type_p, nz_obj * obj_p, const char * string) {
    double * a;
    int n;
    int result;

    a = malloc(sizeof(double) * nz_chunk_size);
    if(a == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    if(string[0] != '{') NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string));
    string += 1;

    for(size_t i = 0; i < nz_chunk_size; i++)
    {
        result = sscanf(string, " %lf %n", &a[i], &n);
        if(result != 1) {
            free(a);
            NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string));
        }
        string += n;
    }

    if(string[0] != '}' || string[1] != '\0') {
        free(a);
        NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string));
    }

    memcpy((double *)obj_p, a, sizeof(double) * nz_chunk_size);
    free(a);

    return NZ_SUCCESS;
}

static nz_rc chunk_type_str_obj(const nz_type * type_p, const nz_obj * obj_p, char ** string) {
    struct strbuf buf;
    char* str = strbuf_alloc(&buf);

    str = strbuf_printf(&buf, str, "{");

    for(size_t i = 0; i < nz_chunk_size; i++) str = strbuf_printf(&buf, str, "%lf ", ((double*)obj_p)[i]);

    buf.len--; // Hack off trailing space
    str = strbuf_printf(&buf, str, "}");

    if(!str) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    *string = str;

    return NZ_SUCCESS;
}
NZ_DECLARE_TYPECLASS(chunk)

// String
NZ_DECLARE_TYPE_ID(string)
NZ_GEN_SIMPLE_TYPE_FNS(string)
NZ_GEN_STATIC_OBJ_FNS(string, sizeof(char *))

static nz_rc string_type_copy_obj(const nz_type * type_p, nz_obj * dst_p, const nz_obj * src_p) {
    char * src = *(char **)(src_p);

    if(!src)
    {
        *(char **)dst_p = NULL;
        return NZ_SUCCESS;
    }

    char * dst;
    dst = strdup(src);

    if(!dst) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    *(char **)dst_p = dst;

    return NZ_SUCCESS;
}

static nz_rc string_type_init_obj(const nz_type * type_p, nz_obj * obj_p, const char * string) {
    free(*(char **)obj_p);
    *(char **)obj_p = NULL;

    char* str = strdup(string);
    if(!str) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    *(char **)obj_p = str;

    return NZ_SUCCESS;
}

static nz_rc string_type_str_obj(const nz_type * type_p, const nz_obj * obj_p, char ** string) {
    char* str = strdup(*(char **)obj_p);

    if(!str) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    *string = str;

    return NZ_SUCCESS;
}

NZ_DECLARE_TYPECLASS(string)

// Array
NZ_DECLARE_TYPE_ID(array)

nz_rc array_type_create_args(nz_type ** type_pp, size_t size, const struct nz_typeclass * typeclass_p, const nz_type * type_p) {
    if(size == 0) NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, strdup("0"));

    struct nz_array_type * array_type_p = malloc(sizeof(struct nz_array_type));
    if(array_type_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    array_type_p->size = size;
    array_type_p->typeclass_p = typeclass_p;
    array_type_p->type_p = type_p;

    *(struct nz_array_type **)type_pp = array_type_p;

    return NZ_SUCCESS;
}

static nz_rc array_type_create(const struct nz_context * context_p, nz_type ** type_pp, const char * string) {
    nz_arg * args[2];
    nz_rc rc = arg_parse("required int n, required generic element_type", string, args);
    if(rc != NZ_SUCCESS) return rc;
    long n = *(long *)args[0];
    char * element_type_str = (char *)args[1];
    free(args[0]);

    const struct nz_typeclass * element_typeclass_p;
    nz_type * element_type_p;

    if(n < 0) {
        free(element_type_str);
        NZ_RETURN_ERR_MSG(NZ_ARG_VALUE, rsprintf("%ld", n));
    }

    rc = nz_context_create_type(context_p, &element_typeclass_p, &element_type_p, element_type_str);
    free(element_type_str);
    if(rc != NZ_SUCCESS) return rc;

    return array_type_create_args(type_pp, n, element_typeclass_p, element_type_p);
}

static void array_type_destroy(nz_type * type_p) {
    free(type_p);
}

static int array_type_is_equal (const nz_type * type_p, const nz_type * other_type_p) {
    struct nz_array_type * array_type_p = (struct nz_array_type *)type_p;
    struct nz_array_type * other_array_type_p = (struct nz_array_type *)other_type_p;

    return (array_type_p->size == other_array_type_p->size &&
            nz_types_are_equal(array_type_p->typeclass_p,       array_type_p->type_p,
                               other_array_type_p->typeclass_p, other_array_type_p->type_p));
}

static nz_rc array_type_create_obj(const nz_type * type_p, nz_obj ** obj_pp) {
    struct nz_array_type * array_type_p = (struct nz_array_type *)type_p;

    nz_obj ** obj_p_array = calloc(2 * array_type_p->size, sizeof(nz_obj *)); // twice the size because second half is shadow array
    if(obj_p_array == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    // The shadow array never has NULLs. Fill it with newly created nz_objs.
    for(size_t i = 0; i < array_type_p->size; i++) {
        array_type_p->typeclass_p->type_create_obj(array_type_p->type_p, &(obj_p_array[i + array_type_p->size]));
    }
    // Leave the first half as NULLs

    *(nz_obj ***)obj_pp = obj_p_array;

    return NZ_SUCCESS;
}

static void array_type_destroy_obj(const nz_type * type_p, nz_obj * obj_p) {
    struct nz_array_type * array_type_p = (struct nz_array_type *)type_p;
    nz_obj ** obj_p_array = (nz_obj **)obj_p;
    for(size_t i = 0; i < array_type_p->size; i++) {
        // We only need to destroy the shadow array because each element in the regular array is either NULL
        // or pointing to its corresponding element in the shadow array.
        array_type_p->typeclass_p->type_destroy_obj(array_type_p->type_p, obj_p_array[i + array_type_p->size]);
    }
    free(obj_p_array);
}

static nz_rc array_type_str(const nz_type * type_p, char ** string) {
    struct nz_array_type * array_type_p = (struct nz_array_type *)type_p;

    char * element_type_str;
    nz_rc rc = array_type_p->typeclass_p->type_str(array_type_p->type_p, &element_type_str);
    if(rc != NZ_SUCCESS) return rc;
    *string = rsprintf("%s<%lu, %s>", array_type_id, array_type_p->size, element_type_str);
    free(element_type_str);
    if(*string == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

static nz_rc array_type_copy_obj(const nz_type * type_p, nz_obj * dst_p, const nz_obj * src_p) {
    struct nz_array_type * array_type_p = (struct nz_array_type *)type_p;
    nz_obj ** src_obj_p_array = (nz_obj **)src_p;
    nz_obj ** dst_obj_p_array = (nz_obj **)dst_p;
    for(size_t i = 0; i < array_type_p->size; i++) {
        if(src_obj_p_array[i] != NULL) { // If the source exists...
            // Copy it to its shadow destination
            array_type_p->typeclass_p->type_copy_obj(array_type_p->type_p, dst_obj_p_array[i + array_type_p->size], src_obj_p_array[i]);
            // And point to it
            dst_obj_p_array[i] = dst_obj_p_array[i + array_type_p->size];
        } else { // If the source doesn't exist...
            // Make the destination NULL
            dst_obj_p_array[i] = NULL;
        }
    }
    return NZ_SUCCESS;
}

static nz_rc array_type_init_obj(const nz_type * type_p, nz_obj * obj_p, const char * string) {
    struct nz_array_type * array_type_p = (struct nz_array_type *)type_p;
    nz_obj ** obj_p_array = (nz_obj **)obj_p;

    size_t len = strlen(string);
    if(string[0] != '{' || string[len-1] != '}') NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string));

    char * elem_list = strndup(&string[1], len - 2);
    if(elem_list == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    const char * pos = elem_list;
    const char * key_start;
    size_t key_length;
    const char * value_start;
    size_t value_length;
    int end;

    for(size_t i = 0; i < array_type_p->size; i++) {
        if(pos == NULL) {
            NZ_ERR_MSG(strdup(elem_list));
            free(elem_list);
            return NZ_ARG_PARSE;
        }
        nz_rc rc = next_arg(elem_list, &pos,
                             &key_start, &key_length,
                             &value_start, &value_length);
        if(rc != NZ_SUCCESS) {
            free(elem_list);
            return rc;
        }
        if(key_start != NULL) {
            free(elem_list);
            NZ_RETURN_ERR(NZ_ARG_PARSE);
        }
        char * element_str = strndup(value_start, value_length);
        if(strcmp(element_str, NZ_NULL_STR) == 0) {
            obj_p_array[i] = NULL;
            free(element_str);
        } else {
            nz_rc rc = array_type_p->typeclass_p->type_init_obj(array_type_p->type_p, obj_p_array[i + array_type_p->size], element_str);
            free(element_str);
            if(rc != NZ_SUCCESS) {
                free(elem_list);
                return rc;
            }
            obj_p_array[i] = obj_p_array[i + array_type_p->size];
        }
    }
    if(pos != NULL) {
        NZ_ERR_MSG(strdup(elem_list));
        free(elem_list);
        return NZ_ARG_PARSE;
    }
    return NZ_SUCCESS;
}

static nz_rc array_type_str_obj(const nz_type * type_p, const nz_obj * obj_p, char ** string) {
    struct nz_array_type * array_type_p = (struct nz_array_type *)type_p;
    nz_obj ** obj_p_array = (nz_obj **)obj_p;
    struct strbuf buf;
    char * str = strbuf_alloc(&buf);

    str = strbuf_printf(&buf, str, "{");

    for(size_t i = 0; i < array_type_p->size; i++) {
        char * elem_string;
        nz_obj * elem = obj_p_array[i];
        if(elem == NULL) {
            str = strbuf_printf(&buf, str, NZ_NULL_STR ", ");
        } else {
            nz_rc rc = array_type_p->typeclass_p->type_str_obj(array_type_p->type_p, elem, &elem_string);
            if(rc != NZ_SUCCESS) {
                free(str);
                return rc;
            }
            str = strbuf_printf(&buf, str, "%s, ", elem_string);
            free(elem_string);
        }
    }

    buf.len -= 2; // Hack off trailing comma and space
    str = strbuf_printf(&buf, str, "}");

    if(!str) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    *string = str;

    return NZ_SUCCESS;
}

NZ_DECLARE_TYPECLASS(array)

// --

