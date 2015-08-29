#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "noise.h"
#include "ntypes.h"
#include "util.h"

object_t * object_alloc(const struct type * type) {
    return type->type_alloc(type);
}

object_t * object_copy(object_t * dst, const object_t * src) {
    if (src == NULL) return NULL;
    if (dst == NULL) return (errno = EINVAL, NULL);
    if (!object_type_compatible(src, dst)) return (errno = EINVAL, NULL);

    return src->object_type->type_copy(dst, src);
}

// Duplicate object
object_t * object_dup(const object_t * src) {
    if (src == NULL) return NULL;

    object_t * dst = object_alloc(src->object_type);
    if (dst == NULL) return NULL;

    if (object_copy(dst, src) == NULL)
        return (object_free(dst), NULL);

    return dst;
}

// object_swap: Copy `src` into `*store`, allocating memory as appropriate
// and returning a ref to a copied version of `src`.
// Attempts to minimize new allocations
// This fn is much more efficent than object_free & object_dup
object_t * object_swap(object_t ** store, const object_t * src) {
    if (src == NULL) return NULL;

    if (*store == NULL) {
        return *store = object_dup(src);
    }

    if (!object_type_compatible(*store, src)) {
        object_free(*store);
        return *store = object_dup(src);
    }
        
    return object_copy(*store, src);
}

void object_free(object_t * obj) {
    if (obj == NULL) return;
    obj->object_type->type_free(obj);
}

char * object_str(object_t * obj) {
    if (obj == NULL) {
        return strdup("null");
    } else if (obj->object_type->type_str) {
        return obj->object_type->type_str(obj);
    } else {
        char * rstr;
        asprintf(&rstr, "<%p instance %p>", obj->object_type, obj);
        return rstr;
    }
}

// ---

object_t * simple_alloc(const struct type * type)
{
    if(type->type_data_size <= 0) return (errno = EINVAL, NULL);

    object_t * obj = calloc(1, sizeof(object_t) + type->type_data_size);
    if(obj == NULL) return NULL;

    obj->object_type = type;
    return obj;
}

void simple_free(object_t * obj) {
    free(obj);
}

object_t * simple_copy(object_t * dst, const object_t * src) {
    assert(object_type_compatible(dst, src));

    memcpy(&dst->object_data, &src->object_data, src->object_type->type_data_size);
    return dst;
}

struct type * make_simple_type(size_t size) {
    if (size <= 0) return (errno = EINVAL, NULL);

    struct type * type = calloc(1, sizeof(*type) + 0);
    if (type == NULL) return (errno = ENOMEM, NULL);

    type->type_data_size = size;
    type->type_alloc = &simple_alloc;
    type->type_copy = &simple_copy;
    type->type_free = &simple_free;
    //type->type_str = &simple_str;
    //type->type_parameters = NULL;
    
    return type;
}

// --

#define VECTOR_INITIAL_CAPACITY 32
#define VECTOR_FACTOR 2

struct vector_data {
    void * vector_contents;
    size_t vector_size;
    size_t vector_capacity;
};

struct vector_parameters {
    size_t param_element_size;
};

object_t * vector_alloc(const struct type * type) {
    struct vector_parameters * params = (struct vector_parameters *) &type->type_parameters;

    object_t * obj = calloc(1, sizeof(*obj) + type->type_data_size);
    if (obj == NULL) return NULL;

    obj->object_type = type;

    struct vector_data * vdata = &CAST_OBJECT(struct vector_data, obj);
    vdata->vector_contents = calloc(VECTOR_INITIAL_CAPACITY, params->param_element_size);
    if (vdata->vector_contents == NULL) return (free(obj), NULL);

    vdata->vector_size = 0;
    vdata->vector_capacity = VECTOR_INITIAL_CAPACITY;

    return obj;
}

void vector_free(object_t * obj) {
    free(CAST_OBJECT(void *, obj));
    free(obj);
}

object_t * vector_copy(object_t * dst, const object_t * src) {
    assert(object_type_compatible(dst, src));

    size_t src_size = vector_get_size(src);
    if (vector_set_size(dst, src_size) != src_size) return NULL;

    struct vector_parameters * params = (struct vector_parameters *) src->object_type->type_parameters;
    memcpy(CAST_OBJECT(void *, dst), CAST_OBJECT(void *, src), src_size * params->param_element_size);

    return dst;
}

struct type * make_vector_type(size_t element_size) {
    if (element_size == 0) return (errno = EINVAL, NULL);

    struct vector_parameters params = {
        .param_element_size = element_size,
    };

    struct type * type = calloc(1, sizeof(*type) + sizeof(params));
    if (type == NULL) return NULL;

    type->type_data_size = sizeof(struct vector_data);
    type->type_alloc = &vector_alloc;
    type->type_copy = &vector_copy;
    type->type_free = &vector_free;
    //type->type_str = &vector_str;
    memcpy(&type->type_parameters, &params, sizeof(params));

    return type;
}

//

// Returns new size, setting errno if != new_size
size_t vector_set_size(object_t * obj, size_t new_size) {
    struct vector_data * vdata = &CAST_OBJECT(struct vector_data, obj);

    if (new_size > vdata->vector_capacity) {
        struct vector_parameters * params = (struct vector_parameters *) &object_type(obj)->type_parameters;
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

size_t vector_get_size(const object_t * obj) {
    return CAST_OBJECT(struct vector_data, obj).vector_size;
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

    object_t * obj = calloc(1, sizeof(object_t) + type->data_size);
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
    type->data_size = length * sizeof(object_t *);
    type->alloc = &array_alloc;
    type->copy = &array_copy;
    type->free = &array_free;
    
    return type;
}
*/

// --- 

void tuple_free(object_t * obj) {
    object_t ** member = CAST_OBJECT(object_t **, obj);
    object_t ** first_member = member;

    while ( ((char *) first_member) + obj->object_type->type_data_size < (char *) member) {
        object_free(*member++);
    }
}

// Shallow copy
// Untested?
object_t * tuple_copy(object_t * dst, const object_t * src) {
    assert(object_type_compatible(dst, src));
    return memcpy(&dst->object_data, &src->object_data, src->object_type->type_data_size);
}

// Untested
object_t * tuple_deep_copy(object_t * dst, const object_t * src) {
    assert(object_type_compatible(dst, src));
    size_t len = tuple_length(src);

    object_t ** src_member = CAST_OBJECT(object_t **, src);
    object_t ** dst_member = CAST_OBJECT(object_t **, dst);

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

struct type * make_tuple_type(size_t length) {
    struct type * type = calloc(1, sizeof(*type));
    if (type == NULL) return NULL;

    type->type_data_size = length * sizeof(object_t *);
    type->type_alloc = &simple_alloc;
    type->type_copy = &tuple_copy;
    type->type_free = &tuple_free;
    //type->type_str = &tuple_str;
    //type->type_parameters = NULL;
    
    return type;
}

// ---

static char * chunk_str (const object_t * obj) {
    double * chunk = &CAST_OBJECT(double, obj);
    return rsprintf("chunk {%f, %f, %f, ... %f} ",
            chunk[0], chunk[1], chunk[2], chunk[noise_chunk_size-1]);
}

static struct type chunk_type = {
    .type_alloc = &simple_alloc,
    .type_copy = &simple_copy,
    .type_free = &simple_free,
    .type_str = &chunk_str,
};

const struct type * get_chunk_type() {
    chunk_type.type_data_size = noise_chunk_size * sizeof(double);
    return &chunk_type;
}

// ---

static char * double_str (const object_t * obj) {
    return rsprintf("%f", CAST_OBJECT(double, obj));
}

static const struct type _double_type = {
    .type_data_size = sizeof(double),
    .type_alloc = &simple_alloc,
    .type_copy = &simple_copy,
    .type_free = &simple_free,
    .type_str = &double_str,
};

const struct type * double_type = &_double_type;

// -

static char * long_str (const object_t * obj)
{
    return rsprintf("%ld", CAST_OBJECT(long, obj));
}

static const struct type _long_type = {
    .type_data_size = sizeof(long),
    .type_alloc = &simple_alloc,
    .type_copy = &simple_copy,
    .type_free = &simple_free,
    .type_str = &long_str,
};

const struct type * long_type = &_long_type;

// -

// Untested: string_type
void string_free(object_t * obj) {
    free(CAST_OBJECT(char *, obj));
    free(obj);
}

object_t * string_copy(object_t * dst, const object_t * src) {
    assert(object_type_compatible(dst, src));

    char ** dst_str = &CAST_OBJECT(char *, dst);
    free(*dst_str);
    *dst_str = strdup(CAST_OBJECT(char *, src));

    if (*dst_str == NULL) return NULL;

    return dst;
}

char * string_str(const object_t * obj) {
    return strdup(CAST_OBJECT(char *, obj));
}

static const struct type _string_type = {
    .type_data_size = sizeof(char *),
    .type_alloc = &simple_alloc,
    .type_copy = &string_copy,
    .type_free = &string_free,
    .type_str = &string_str
};

const struct type * string_type = &_string_type;
