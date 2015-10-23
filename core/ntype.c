#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "noise.h"
#include "core/ntype.h"
#include "core/note.h"
#include "core/util.h"

static int types_init();

// --

#define GEN_STATIC_OBJ_FNS(NAME, CTYPE) \
static nz_obj_p type_create_ ## NAME (const struct nz_type * type) { \
    return calloc(1, sizeof(CTYPE)); \
} \
static void type_destroy_ ## NAME (const struct nz_type * type, nz_obj_p obj_p) { \
    return free(obj_p); \
} \
static int type_copy_ ## NAME (const struct nz_type * type, nz_obj_p dst_p, const nz_obj_p src_p) { \
    memcpy(dst_p, src_p, sizeof(CTYPE)); \
    return 0; \
}

#define GEN_PRIMITIVE_STRING_FNS(NAME, CTYPE, FORMAT_STR) \
static char * type_str_ ## NAME (const struct nz_type * type, const nz_obj_p obj_p) \
{ \
    return rsprintf(FORMAT_STR, *((CTYPE)*)obj_p); \
}

#define DECLARE_TYPE(NAME) \
const struct nz_type_ ## NAME { \
    struct nz_type; \
} nz_type_ ## NAME = {{ \
    .type_create = &type_create_ ## NAME, \
    .type_destroy = &type_destroy_ ## NAME, \
    .type_copy = &type_copy_ ## NAME, \
    .type_str = &type_str_ ## NAME, \
}};

#define DECLARE_PRIMITIVE_TYPE(CTYPE, FORMAT_STR) \
GEN_STATIC_OBJ_FNS(CTYPE, CTYPE) \
GEN_PRIMITIVE_STRING_FNS(CTYPE, CTYPE, FORMAT_STR) \
GEN_STATIC_OBJ_FNS(CTYPE)

// --

#define VECTOR_INITIAL_CAPACITY 32
#define VECTOR_FACTOR 2

struct vector_data {
    void * vector_contents;
    size_t vector_size;
    size_t vector_capacity;
};

struct vector_type {
    const struct nz_type;
    size_t param_element_size;
};

nz_obj_p vector_create(const struct nz_type * type) {
    struct vector_parameters * params = (struct vector_parameters *) &type->type_parameters;

    nz_obj_p obj = calloc(1, type->type_size);
    if (obj == NULL) return NULL;

    struct vector_data * vdata = &NZ_CAST(struct vector_data, obj);
    vdata->vector_contents = calloc(VECTOR_INITIAL_CAPACITY, params->param_element_size);
    if (vdata->vector_contents == NULL) return (free(obj), NULL);

    vdata->vector_size = 0;
    vdata->vector_capacity = VECTOR_INITIAL_CAPACITY;

    return obj;
}

void vector_destroy(nz_obj_p obj) {
    free(NZ_CAST(void *, obj));
    free(obj);
}

nz_obj_p vector_copy(nz_obj_p dst, const nz_obj_p src) {
    if (!nz_obj_type_compatible(dst, src)) return (errno = EINVAL, NULL);

    size_t src_size = nz_vector_get_size(src);
    if (nz_vector_set_size(dst, src_size) != src_size) return NULL;

    struct vector_parameters * params = (struct vector_parameters *) src->obj_type->type_parameters;
    memcpy(NZ_CAST(void *, dst), NZ_CAST(void *, src), src_size * params->param_element_size);

    return dst;
}

struct nz_type * nz_type_create_vector(size_t element_size) {
    if (element_size == 0) return (errno = EINVAL, NULL);

    struct vector_parameters params = {
        .param_element_size = element_size,
    };

    struct nz_type * type = calloc(1, sizeof(*type) + sizeof(params));
    if (type == NULL) return NULL;

    type->type_size = sizeof(struct vector_data);
    type->type_create = &vector_create;
    type->type_copy = &vector_copy;
    type->type_destroy = &vector_destroy;
    //type->type_str = &vector_str;
    memcpy(&type->type_parameters, &params, sizeof(params));

    return type;
}

//

// Returns new size, setting errno if != new_size
size_t nz_vector_set_size(nz_obj_p obj, size_t new_size) {
    struct vector_data * vdata = &NZ_CAST(struct vector_data, obj);

    if (new_size > vdata->vector_capacity) {
        struct vector_parameters * params = (struct vector_parameters *) &obj->obj_type->type_parameters;
        size_t el_size = params->param_element_size;

        size_t new_capacity = vdata->vector_capacity;
        while (new_capacity && new_capacity < new_size) new_capacity *= VECTOR_FACTOR;

        if (!new_capacity) return (errno = EOVERFLOW, vdata->vector_size);

        void * new_ptr = realloc(vdata->vector_contents, new_capacity * el_size);
        if (new_ptr == NULL) return (errno = ENOMEM, vdata->vector_size);

        vdata->vector_contents = new_ptr;
        vdata->vector_capacity = new_capacity;
    }

    return vdata->vector_size = new_size;
}

size_t nz_vector_get_size(const nz_obj_p obj) {
    return NZ_CAST(struct vector_data, obj).vector_size;
}

int nz_vector_push_back(nz_obj_p vec, void * data) {
    struct vector_data * vdata = &NZ_CAST(struct vector_data, vec);
    size_t size = vdata->vector_size + 1;
    if (nz_vector_set_size(vec, size) != size) return -1;

    struct vector_parameters * params = (struct vector_parameters *) &vec->obj_type->type_parameters;
    size_t el_size = params->param_element_size;

    char * dst = &NZ_CAST(char *, vec)[(size - 1) * el_size];
    memcpy(dst, data, el_size);

    return 0;
}

void * nz_vector_at(nz_obj_p vec, size_t idx) {
    if (idx >= nz_vector_get_size(vec)) 
        return (errno = EINVAL, NULL);

    struct vector_parameters * params = (struct vector_parameters *) &vec->obj_type->type_parameters;
    size_t el_size = params->param_element_size;

    char * el = &NZ_CAST(char *, vec)[idx * el_size];
    return (void *) el;
}

void nz_vector_erase(nz_obj_p vec, size_t idx) {
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

size_t nz_vector_sizeofel(nz_obj_p vec) {
    struct vector_parameters * params = (struct vector_parameters *) &vec->obj_type->type_parameters;
    return params->param_element_size;
}

// ---

/*
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
    object_t ** member = (object_t **) obj->object_data;

    for (size_t i = 0; i < params->length; i++)
    {
        object_free(member[i]);
    }
}

error_t array_copy(object_t * dst, const object_t * src)
{
    if (src->object_type != dst->object_type) return ERR_INVALID;

    array_parameters_t * params = (array_parameters_t *) src->object_type->parameters;
    object_t ** src_member = (object_t **) src->object_data;
    object_t ** dst_member = (object_t **) dst->object_data;

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
*/

// --- 
// Consider getting rid of this? XXX Vector<object_t *> instead?

/*
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
    return memcpy(&dst->object_data, &src->object_data, src->object_type->type_size);
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

struct nz_type * make_tuple_type(size_t length) {
    struct nz_type * type = calloc(1, sizeof(*type));
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

// ---

DECLARE_PRIMITIVE_TYPE(float, "%f")
DECLARE_PRIMITIVE_TYPE(double, "%lf")
DECLARE_PRIMITIVE_TYPE(int, "%d")
DECLARE_PRIMITIVE_TYPE(long, "%ld")

// -

// Untested: string_type
void string_destroy(nz_obj_p obj) {
    free(NZ_CAST(char *, obj));
    free(obj);
}

nz_obj_p string_copy(nz_obj_p dst, const nz_obj_p src) {
    assert(nz_obj_type_compatible(dst, src));

    char ** dst_str = &NZ_CAST(char *, dst);
    free(*dst_str);
    *dst_str = strdup(NZ_CAST(char *, src));

    if (*dst_str == NULL) return NULL;

    return dst;
}

char * string_str(const nz_obj_p obj) {
    return strdup(NZ_CAST(char *, obj));
}

const struct nz_type nz_string_type[1] = {{
    .type_size = sizeof(char *),
    .type_create = &nz_simple_create_,
    .type_destroy = &string_destroy,
    .type_copy = &string_copy,
    .type_str = &string_str
}};

// Other types!

static char * chunk_str (const nz_obj_p obj) {
    double * chunk = &NZ_CAST(double, obj);
    return rsprintf("chunk {%f, %f, %f, ... %f} ",
            chunk[0], chunk[1], chunk[2], chunk[nz_chunk_size-1]);
}

struct nz_type nz_chunk_type[1] = {{
    .type_create = &nz_simple_create_,
    .type_destroy = &nz_simple_destroy_,
    .type_copy = &nz_simple_copy_,
    .type_str = &chunk_str,
}};

struct nz_type * nz_sample_type = NULL;
struct nz_type * nz_object_vector_type = NULL;
struct nz_type * nz_note_vector_type = NULL;
struct nz_type * nz_vector_vector_type = NULL;

// Some types can't be statically created (easily), so build at runtime
static int types_init() {
    // Chunk type (fixed-size audio buffer)
    nz_chunk_type->type_size = nz_chunk_size * sizeof(double);

    // Sample type (variable-length recording)
    if (nz_sample_type == NULL)
        nz_sample_type = nz_type_create_vector(sizeof(double)); 

    // Note vector type
    if (nz_note_vector_type == NULL)
    nz_note_vector_type = nz_type_create_vector(sizeof(struct nz_note));

    // Object vector type
    if (nz_object_vector_type == NULL)
    nz_object_vector_type = nz_type_create_vector(sizeof(nz_obj_p));

    if (nz_sample_type == NULL || nz_object_vector_type == NULL)
        return -1;

    return 0;
}
