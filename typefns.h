#ifndef __TYPEFNS_H
#define __TYPEFNS_H

#include <stddef.h>
#include "error.h"

// Stuff relating to types
struct type;
struct object;

typedef void * type_parameters_pt;

typedef error_t (*type_copy_fn_pt)(struct object * dst, const struct object * src);
typedef struct object * (*type_alloc_fn_pt)(const struct type * type);
typedef void (*type_free_fn_pt)(struct object * obj);
typedef char * (*type_str_fn_pt)(struct object * obj);

#define VARIABLE_SIZE 0

typedef struct type
{
    type_parameters_pt parameters;
    size_t data_size;
    type_alloc_fn_pt alloc;
    type_copy_fn_pt copy;
    type_free_fn_pt free;
    type_str_fn_pt str;
} type_t;

typedef struct object
{
    const type_t * object_type;
    char object_data[0];
} object_t;


//

object_t * object_alloc(const type_t * type);
error_t object_copy(object_t * dst, const object_t * src);
object_t * object_dup(const object_t * src);
object_t * object_swap(object_t ** store, const object_t * src);
void object_free(object_t * obj);
char * object_str(object_t * obj);

static inline const type_t * object_type(object_t * object) {
    return object->object_type;
}

//

type_t * get_chunk_type();
type_t * make_simple_type(size_t size);
type_t * make_array_type(size_t length, const type_t * element_type);
type_t * make_tuple_type(size_t length);
type_t * make_object_and_pod_type(size_t total_size);

//

object_t * simple_alloc(const type_t * type);
void simple_free(object_t * obj);
error_t simple_copy(object_t * dst, const object_t * src);

static inline size_t tuple_length(object_t * object) {
    return object->object_type->data_size / sizeof(object_t *);
}

//
#define CAST_OBJECT(type, obj) (*(type *)(obj)->object_data)
#define CAST_TUPLE(type, idx, obj) CAST_OBJECT(type, (&CAST_OBJECT(object_t *, (obj)))[(idx)])

extern type_t * double_type;
extern type_t * long_type;

// nb: there is no way to 'destroy' or 'free' types once created.

#endif
