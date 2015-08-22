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

#define VARIABLE_SIZE 0

typedef struct type
{
    type_parameters_pt parameters;
    size_t data_size;
    type_alloc_fn_pt alloc;
    type_copy_fn_pt copy;
    type_free_fn_pt free;
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
void object_free(object_t * obj);

//

type_t * get_chunk_type();

typedef struct
{
    size_t size;
} simple_parameters_t;

type_t * make_simple_type(size_t size);

typedef struct
{
    size_t length;
    const type_t * element_type;
} array_parameters_t;

type_t * make_array_type(size_t length, const type_t * element_type);

//
#define CAST_OBJECT(type, obj) (*(type *)(obj)->object_data)

extern type_t * double_type;
extern type_t * long_type;

// nb: there is no way to 'destroy' or 'free' types once created.

#endif
