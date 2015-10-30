#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "noise.h"
#include "core/context.h"
#include "core/ntype.h"
#include "core/util.h"

struct nz_array_type {
    size_t size;
    const struct nz_typeclass * typeclass_p;
    const nz_type * type_p;
};

nz_rc array_type_create_args(nz_type ** type_pp, size_t size, const struct nz_typeclass * typeclass_p, const nz_type * type_p);

nz_rc nz_init_type_system(struct nz_context * context_p) {
    context_p->n_registered_typeclasses = 0;
    context_p->registered_typeclass_capacity = 16;
    context_p->registered_typeclasses = calloc(context_p->registered_typeclass_capacity, sizeof(struct nz_typeclass *));
    if(context_p->registered_typeclasses == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    return NZ_SUCCESS;
}

nz_rc nz_register_typeclass(struct nz_context * context_p, struct nz_typeclass const * typeclass_p) {
    if(context_p->n_registered_typeclasses < context_p->registered_typeclass_capacity) {
        context_p->registered_typeclass_capacity *= 2;
        struct nz_typeclass const ** newptr =  realloc(context_p->registered_typeclasses, context_p->registered_typeclass_capacity * sizeof(struct nz_typeclass *));
        if(newptr == NULL) {
            free(context_p->registered_typeclasses);
            NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
        }
        context_p->registered_typeclasses = newptr;
    }

    context_p->registered_typeclasses[context_p->n_registered_typeclasses] = typeclass_p;
    context_p->n_registered_typeclasses++;

    return NZ_SUCCESS;
}

void nz_deinit_type_system(struct nz_context * context_p) {
    free(context_p->registered_typeclasses);
}

int nz_types_are_equal(const struct nz_typeclass * typeclass_p,       const nz_type * type_p,
                       const struct nz_typeclass * other_typeclass_p, const nz_type * other_type_p)
{
    if(strcmp(typeclass_p->type_id, other_typeclass_p->type_id)) return 0;
    return typeclass_p->type_is_equal(type_p, other_type_p);
}

nz_rc nz_type_create(const struct nz_context * context_p, const struct nz_typeclass ** typeclass_pp, nz_type ** type_pp, const char * string) {
    for(size_t i = 0; i < context_p->n_registered_typeclasses; i++) {
        size_t c = 0;
        const char * type_id = context_p->registered_typeclasses[i]->type_id;
        size_t len = strlen(string);
        for(;;) {
            if(type_id[c] == '\0') {
                if(string[c] == '\0') {
                    // Match, no args
                    *typeclass_pp = context_p->registered_typeclasses[i];
                    return (*typeclass_pp)->type_create(context_p, type_pp, NULL);
                } else if(string[c] == '<' && string[len - 1] == '>') {
                    // Match, args
                    char * args = strndup(&(string[c + 1]), len - 2 - c);
                    if(args == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
                    *typeclass_pp = context_p->registered_typeclasses[i];
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

enum arg_rc {SUCCESS = 0, UNMATCHED_CLOSE, UNMATCHED_OPEN, UNMATCHED_QUOTE};

static enum arg_rc next_arg(const char * string, const char ** pos, const char ** start, size_t * length, char open, char close)
{
    int angle_brackets = 0;
    size_t trailing_whitespace = 0;

    enum {
        SKIP_STARTING_WHITESPACE,
        ARGUMENT,
        IN_STRING,
        IN_STRING_ESCAPE,
    } state = SKIP_STARTING_WHITESPACE;

    for(;;) {
        switch(state) {
            case SKIP_STARTING_WHITESPACE:
                switch(**pos) {
                    case ' ':
                        break;
                    case '"':
                        *start = *pos;
                        state = IN_STRING;
                        break;
                    case ',':
                        *start = *pos;
                        *length = 0;
                        (*pos)++;
                        return SUCCESS;
                    case '\0':
                        *start = *pos;
                        *length = 0;
                        *pos = NULL;
                        return SUCCESS;
                    default:
                        if(**pos == open) {
                            *start = *pos;
                            angle_brackets = 1;
                            state = ARGUMENT;
                            break;
                        } else if(**pos == close) {
                            return UNMATCHED_CLOSE;
                        }
                        *start = *pos;
                        state = ARGUMENT;
                        break;
                }
                break;
            case ARGUMENT:
                switch(**pos) {
                    case '"':
                        state = IN_STRING;
                        break;
                    case ',':
                        if(angle_brackets == 0) {
                            *length = *pos - *start - trailing_whitespace;
                            (*pos)++;
                            return SUCCESS;
                        } else return UNMATCHED_OPEN;
                    case '\0':
                        if(angle_brackets == 0) {
                            *length = *pos - *start - trailing_whitespace;
                            *pos = NULL;
                            return SUCCESS;
                        } else return UNMATCHED_OPEN;
                    default:
                        if(**pos == open) {
                            angle_brackets++;
                            break;
                        } else if(**pos == close) {
                            if(angle_brackets == 0) return UNMATCHED_CLOSE;
                            angle_brackets--;
                            break;
                        }
                }
                break;
            case IN_STRING:
                switch(**pos) {
                    case '"':
                        state = ARGUMENT;
                        break;
                    case '\\':
                        state = IN_STRING_ESCAPE;
                        break;
                    case '\0':
                        return UNMATCHED_QUOTE;
                }
                break;
            case IN_STRING_ESCAPE:
                switch(**pos) {
                    case '\0':
                        return UNMATCHED_QUOTE;
                    default:
                        state = IN_STRING;
                        break;
                }
                break;
        }

        if(**pos == ' ') {
            trailing_whitespace++;
        } else {
            trailing_whitespace = 0;
        }
        (*pos)++;
    }
}

nz_rc nz_next_type_arg(const char * string, const char ** pos, const char ** start, size_t * length) {
    if(next_arg(string, pos, start, length, '<', '>') != SUCCESS) NZ_RETURN_ERR_MSG(NZ_TYPE_ARG_PARSE, strdup(string));
    return NZ_SUCCESS;
}

nz_rc nz_next_list_arg(const char * string, const char ** pos, const char ** start, size_t * length) {
    if(next_arg(string, pos, start, length, '{', '}') != SUCCESS) NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    return NZ_SUCCESS;
}

// --
// C Primitives

DECLARE_PRIMITIVE_TYPECLASS(int, int, "%d")
DECLARE_PRIMITIVE_TYPECLASS(long, long, "%ld")
DECLARE_PRIMITIVE_TYPECLASS(real, double, "%lf")

// --
// Simple noise types

// Chunk
GEN_SIMPLE_TYPE_FNS(chunk)
GEN_STATIC_OBJ_FNS(chunk, (sizeof(double) * nz_chunk_size)) // TODO: malloc might result in unaligned??
GEN_SHALLOW_COPY_FN(chunk, (sizeof(double) * nz_chunk_size))

static nz_rc chunk_type_init_obj(const nz_type * type_p, nz_obj * obj_p, const char * string) {
    double * a;
    int n;
    int result;

    a = malloc(sizeof(double) * nz_chunk_size);
    if(a == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    if(string[0] != '{') NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    string += 1;

    for(size_t i = 0; i < nz_chunk_size; i++)
    {
        result = sscanf(string, " %lf %n", &a[i], &n);
        if(result != 1) {
            free(a);
            NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
        }
        string += n;
    }

    if(string[0] != '}' || string[1] != '\0') {
        free(a);
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
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
DECLARE_TYPECLASS(chunk)

// String
GEN_SIMPLE_TYPE_FNS(string)
GEN_STATIC_OBJ_FNS(string, sizeof(char *))

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

DECLARE_TYPECLASS(string)

// Array
nz_rc array_type_create_args(nz_type ** type_pp, size_t size, const struct nz_typeclass * typeclass_p, const nz_type * type_p)
{
    if(size == 0) NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_VALUE, strdup("0"));

    struct nz_array_type * array_type_p = malloc(sizeof(struct nz_array_type));
    if(array_type_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    array_type_p->size = size;
    array_type_p->typeclass_p = typeclass_p;
    array_type_p->type_p = type_p;

    *(struct nz_array_type **)type_pp = array_type_p;

    return NZ_SUCCESS;
}

static nz_rc array_type_create(const struct nz_context * context_p, nz_type ** type_pp, const char * string) {
    if(string == NULL) NZ_RETURN_ERR(NZ_EXPECTED_TYPE_ARGS);

    const char * pos = string;
    const char * start;
    size_t length;
    int end;
    nz_rc rc;

    rc = nz_next_type_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * n_elements_str = strndup(start, length);
    size_t n_elements;
    if(sscanf(n_elements_str, "%lu%n", &n_elements, &end) != 1 || end <= 0 || (size_t)end != length) {
        free(n_elements_str);
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    }
    free(n_elements_str);

    if(pos == NULL) NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));

    rc = nz_next_type_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * type_str = strndup(start, length);
    const struct nz_typeclass * element_typeclass_p;
    nz_type * element_type_p;
    rc = nz_type_create(context_p, &element_typeclass_p, &element_type_p, type_str);
    free(type_str);
    if(rc != NZ_SUCCESS) return rc;

    if(pos != NULL) NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));

    return array_type_create_args(type_pp, n_elements, element_typeclass_p, element_type_p);
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
    if(string[0] != '{' || string[len-1] != '}') NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));

    char * elem_list = strndup(&string[1], len - 2);
    if(elem_list == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    const char * pos = elem_list;
    const char * start;
    size_t length;
    int end;

    for(size_t i = 0; i < array_type_p->size; i++) {
        if(pos == NULL) {
            NZ_ERR_MSG(strdup(elem_list));
            free(elem_list);
            return NZ_OBJ_ARG_PARSE;
        }
        nz_rc rc = nz_next_list_arg(elem_list, &pos, &start, &length);
        if(rc != NZ_SUCCESS) {
            free(elem_list);
            return rc;
        }
        char * element_str = strndup(start, length);
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
        return NZ_OBJ_ARG_PARSE;
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

DECLARE_TYPECLASS(array)

// --

nz_rc nz_init_types(struct nz_context * context_p) {
    nz_rc rc;
    rc = nz_register_typeclass(context_p, &nz_int_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_register_typeclass(context_p, &nz_long_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_register_typeclass(context_p, &nz_real_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_register_typeclass(context_p, &nz_chunk_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_register_typeclass(context_p, &nz_string_typeclass); if(rc != NZ_SUCCESS) return rc;
    rc = nz_register_typeclass(context_p, &nz_array_typeclass); if(rc != NZ_SUCCESS) return rc;
    return NZ_SUCCESS;
}

// OLD SHIT

/*
#define VECTOR_INITIAL_CAPACITY 32
#define VECTOR_FACTOR 2

struct vector {
    void * vector_contents;
    size_t vector_size;
    size_t vector_capacity;
};

struct vector_type {
    const struct nz_typeclass;
    size_t param_element_size;
};

nz_obj * vector_create(const struct nz_typeclass * type) {
    struct vector_parameters * params = (struct vector_parameters *) &type->type_parameters;

    nz_obj * obj = calloc(1, type->type_size);
    if (obj == NULL) return NULL;

    struct vector * vdata = &NZ_CAST(struct vector, obj);
    vdata->vector_contents = calloc(VECTOR_INITIAL_CAPACITY, params->param_element_size);
    if (vdata->vector_contents == NULL) return (free(obj), NULL);

    vdata->vector_size = 0;
    vdata->vector_capacity = VECTOR_INITIAL_CAPACITY;

    return obj;
}

void vector_destroy(nz_obj * obj) {
    free(NZ_CAST(void *, obj));
    free(obj);
}

nz_obj * vector_copy(nz_obj * dst, const nz_obj * src) {
    if (!nz_obj_type_compatible(dst, src)) return (errno = EINVAL, NULL);

    size_t src_size = nz_vector_get_size(src);
    if (nz_vector_set_size(dst, src_size) != src_size) return NULL;

    struct vector_parameters * params = (struct vector_parameters *) src->obj_type->type_parameters;
    memcpy(NZ_CAST(void *, dst), NZ_CAST(void *, src), src_size * params->param_element_size);

    return dst;
}

struct nz_typeclass * nz_type_create_vector(size_t element_size) {
    if (element_size == 0) return (errno = EINVAL, NULL);

    struct vector_parameters params = {
        .param_element_size = element_size,
    };

    struct nz_typeclass * type = calloc(1, sizeof(*type) + sizeof(params));
    if (type == NULL) return NULL;

    type->type_size = sizeof(struct vector);
    type->type_create = &vector_create;
    type->type_copy = &vector_copy;
    type->type_destroy = &vector_destroy;
    //type->type_str = &vector_str;
    memcpy(&type->type_parameters, &params, sizeof(params));

    return type;
}

//

// Returns new size, setting errno if != new_size
size_t nz_vector_set_size(nz_obj * obj, size_t new_size) {
    struct vector * vdata = &NZ_CAST(struct vector, obj);

    if (new_size > vdata->vector_capacity) {
        struct vector_parameters * params = (struct vector_parameters *) &obj->obj_type->type_parameters;
        size_t el_size = params->param_element_size;

        size_t new_capacity = vdata->vector_capacity;
        while (new_capacity && new_capacity < new_size) new_capacity *= VECTOR_FACTOR;

        if (!new_capacity) return (errno = EOVERFLOW, vdata->vector_size);

        void * new_ptr = realloc(vdata->vector_contents, new_capacity * el_size);
        if (new_ptr == NULL) return (errno = ENOT_ENOUGH_MEMORY, vdata->vector_size);

        vdata->vector_contents = new_ptr;
        vdata->vector_capacity = new_capacity;
    }

    return vdata->vector_size = new_size;
}

size_t nz_vector_get_size(const nz_obj * obj) {
    return NZ_CAST(struct vector, obj).vector_size;
}

int nz_vector_push_back(nz_obj * vec, void * data) {
    struct vector * vdata = &NZ_CAST(struct vector, vec);
    size_t size = vdata->vector_size + 1;
    if (nz_vector_set_size(vec, size) != size) return -1;

    struct vector_parameters * params = (struct vector_parameters *) &vec->obj_type->type_parameters;
    size_t el_size = params->param_element_size;

    char * dst = &NZ_CAST(char *, vec)[(size - 1) * el_size];
    memcpy(dst, data, el_size);

    return 0;
}

void * nz_vector_at(nz_obj * vec, size_t idx) {
    if (idx >= nz_vector_get_size(vec)) 
        return (errno = EINVAL, NULL);

    struct vector_parameters * params = (struct vector_parameters *) &vec->obj_type->type_parameters;
    size_t el_size = params->param_element_size;

    char * el = &NZ_CAST(char *, vec)[idx * el_size];
    return (void *) el;
}

void nz_vector_erase(nz_obj * vec, size_t idx) {
    size_t size = nz_vector_get_size(vec);
    if (idx >= size) return;
    size--;

    struct vector_parameters * params = (struct vector_parameters *) &vec->obj_type->type_parameters;
    size_t el_size = params->param_element_size;

    char * el = &NZ_CAST(char *, vec)[idx * el_size];
    memmove(el, el + el_size, el_size * (size - idx));

    // This shouldn't fail because we're making it smaller
    assert(nz_vector_set_size(vec, size) == size);
}

size_t nz_vector_sizeofel(nz_obj * vec) {
    struct vector_parameters * params = (struct vector_parameters *) &vec->obj_type->type_parameters;
    return params->param_element_size;
}

// ---

typedef struct
{
    size_t length;
    const type_t * element_type;
} array_parameters_t;

object_t * array_alloc(const type_t * type)
{
    //array_parameters_t * params = (array_parameters_t *) type->parameters;

    object_t * obj = calloc(1, sizeof(object_t) + type->size);
    if(obj == NULL) return NULL;

    obj->object_type = type;
    return obj;

}

void array_free(object_t * obj)
{
    array_parameters_t * params = (array_parameters_t *) obj->object_type->parameters;
    object_t ** member = (object_t **) obj->object;

    for (size_t i = 0; i < params->length; i++)
    {
        object_free(member[i]);
    }
}

error_t array_copy(object_t * dst, const object_t * src)
{
    if (src->object_type != dst->object_type) return ERR_INVALID;

    array_parameters_t * params = (array_parameters_t *) src->object_type->parameters;
    object_t ** src_member = (object_t **) src->object;
    object_t ** dst_member = (object_t **) dst->object;

    error_t r = SUCCESS;

    for (size_t i = 0; i < params->length; i++)
    {
        if (*src_member == NULL){
            if (*dst_member == NULL) {

            } else {
                object_free(*dst_member);
                *dst_member = NULL;
            }
        } else { 
            if (*dst_member == NULL) {
                *dst_member = object_dup(*src_member);
                if (*dst_member == NULL) r |= ERR_MALLOC;
            } else {
                r |= object_copy(*dst_member, *src_member);
            }
        }

        dst_member++;
        src_member++;
    }

    return r;
}

type_t * make_array_type(size_t length, const type_t * element_type)
{
    //if (length == 0) return NULL; // do we want this? XXX

    type_t * type = calloc(1, sizeof(type_t));
    if (type == NULL) return NULL;

    type->parameters = calloc(1, sizeof(array_parameters_t));
    if (type->parameters == NULL) return free(type), NULL;

    array_parameters_t * params = (array_parameters_t *) type->parameters;

    params->length = length;
    params->element_type = element_type;
    type->size = length * sizeof(object_t *);
    type->alloc = &array_alloc;
    type->copy = &array_copy;
    type->free = &array_free;
    
    return type;
}

// --- 
// Consider getting rid of this? XXX Vector<object_t *> instead?

void tuple_free(object_t * obj) {
    object_t ** member = NZ_CAST(object_t **, obj);
    object_t ** first_member = member;

    while ( ((char *) first_member) + obj->object_type->type_size < (char *) member) {
        object_free(*member++);
    }
}

// Shallow copy
// Untested?
object_t * tuple_copy(object_t * dst, const object_t * src) {
    assert(object_type_compatible(dst, src));
    return memcpy(&dst->object, &src->object, src->object_type->type_size);
}

// Untested
object_t * tuple_deep_copy(object_t * dst, const object_t * src) {
    assert(object_type_compatible(dst, src));
    size_t len = tuple_length(src);

    object_t ** src_member = NZ_CAST(object_t **, src);
    object_t ** dst_member = NZ_CAST(object_t **, dst);

    for (size_t i = 0; i < len; i++) {
        if (src_member[i] == NULL) {
            object_free(dst_member[i]);
        } if (object_type_compatible(src_member[i], dst_member[i])) {
            object_copy(dst_member[i], src_member[i]);
        } else {
            object_free(dst_member[i]);
            dst_member[i] = object_dup(src_member[i]);
        }
        if (dst_member[i] == NULL && src_member[i] != NULL)
            return NULL;
    }

    return memcpy(dst, src, sizeof(*src));
}

struct nz_typeclass * make_tuple_type(size_t length) {
    struct nz_typeclass * type = calloc(1, sizeof(*type));
    if (type == NULL) return NULL;

    type->type_size = length * sizeof(object_t *);
    type->type_create = &nz_simple_create_;
    type->type_copy = &tuple_copy;
    type->type_destroy = &tuple_free;
    //type->type_str = &tuple_str;
    //type->type_parameters = NULL;
    
    return type;
}
*/
