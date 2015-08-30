#ifndef __TYPEFNS_H
#define __TYPEFNS_H

#include <stddef.h>

#include "error.h"

// Stuff relating to types
struct type;
struct object;

typedef struct object * (*type_copy_fn_pt)(struct object * dst, const struct object * src);
typedef struct object * (*type_alloc_fn_pt)(const struct type * type);
typedef void (*type_free_fn_pt)(struct object * obj);
typedef char * (*type_str_fn_pt)(const struct object * obj);

struct type {
    size_t type_data_size;
    type_alloc_fn_pt type_alloc;
    type_copy_fn_pt type_copy;
    type_free_fn_pt type_free;
    type_str_fn_pt type_str;
    char type_parameters[0];
};

typedef struct object {
    const struct type * object_type;
    char object_data[0];
} object_t;

//

object_t * object_alloc(const struct type * type);
object_t * object_copy(object_t * dst, const object_t * src);
object_t * object_dup(const object_t * src);
object_t * object_swap(object_t ** store, const object_t * src);
void object_free(object_t * obj);
char * object_str(object_t * obj);

static inline const struct type * object_type(const object_t * object) {
    return object->object_type;
}

//

struct type * make_simple_type(size_t size);
//struct type * make_tuple_type(size_t length);
struct type * make_vector_type(size_t element_size);
// TODO Array type is untested, needs some more thought...
//struct type * make_array_type(size_t length, const struct type * element_type);

static inline int type_compatible(const struct type * ta, const struct type * tb) {
    // This might change if the type system gets more complicated
    return ta == NULL || tb == NULL || ta == tb;
}

static inline int object_type_compatible(const object_t * oa, const object_t * ob) {
    return (oa != NULL && ob != NULL) && type_compatible(object_type(oa), object_type(ob));
}

//

object_t * simple_alloc(const struct type * type);
void simple_free(object_t * obj);
object_t * simple_copy(object_t * dst, const object_t * src);

/*
static inline size_t tuple_length(const object_t * object) {
    return object->object_type->type_data_size / sizeof(object_t *);
}
*/

size_t vector_set_size(object_t * obj, size_t new_size);
size_t vector_get_size(const object_t * obj);

// 
#define CAST_OBJECT(type, obj) (*((type *)((union { char * c; type* x;})(char *)((obj)->object_data)).x))
#define CAST_TUPLE(type, idx, obj) CAST_OBJECT(type, (&CAST_OBJECT(object_t *, (obj)))[(idx)])

extern const struct type * double_type;
extern const struct type * long_type;
extern const struct type * string_type;
const struct type * get_chunk_type();
const struct type * get_sample_type();
const struct type * get_object_vector_type();

// nb: there is no way to 'destroy' or 'free' types once created.

#endif
