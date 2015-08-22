#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"
#include "typefns.h"
#include "util.h"

object_t * object_alloc(const type_t * type)
{
    //if (type == NULL) return NULL;

    return type->alloc(type);
}

error_t object_copy(object_t * dst, const object_t * src)
{
    //if (src == NULL || dst == NULL) return ERR_INVALID;
    if (src->object_type != dst->object_type) return ERR_INVALID;

    return src->object_type->copy(dst, src);
}

object_t * object_dup(const object_t * src)
{
    if (src == NULL) return NULL;

    object_t * dst = src->object_type->alloc(src->object_type);
    if(dst == NULL) return NULL;

    error_t r = object_copy(dst, src);
    if(r != SUCCESS) {
        object_free(dst);    
        return NULL;
    }

    return dst;
}

void object_free(object_t * obj)
{
    if (obj == NULL) return;

    obj->object_type->free(obj);
}

char * object_str(object_t * object) {
    if (object->object_type->str) {
        return object->object_type->str(object);
    } else {
        char * rstr;
        asprintf(&rstr, "<%p instance %p>", object->object_type, object);
        return rstr;
    }
}

// ---

object_t * simple_alloc(const type_t * type)
{
    if(type->data_size == VARIABLE_SIZE) return NULL;

    object_t * obj = calloc(1, sizeof(object_t) + type->data_size);
    if(obj == NULL) return NULL;

    obj->object_type = type;
    return obj;
}

void simple_free(object_t * obj)
{
    free(obj);
}

error_t simple_copy(object_t * dst, const object_t * src)
{
    if (src->object_type != dst->object_type) return ERR_INVALID;

    //simple_parameters_t * params = (simple_parameters_t *) src->object_type->parameters;

    memcpy(&dst->object_data, &src->object_data, src->object_type->data_size);
    return SUCCESS;
}

type_t * make_simple_type(size_t size)
{
    if (size == VARIABLE_SIZE) return NULL;

    type_t * type = calloc(1, sizeof(type_t));
    if (type == NULL) return NULL;

    type->parameters = NULL;
    type->data_size = size;
    type->alloc = &simple_alloc;
    type->copy = &simple_copy;
    type->free = &simple_free;
    
    return type;
}

// ---
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
    type->data_size = length * sizeof(object_t **);
    type->alloc = &array_alloc;
    type->copy = &array_copy;
    type->free = &array_free;
    
    return type;
}

// ---
size_t global_chunk_size = 128;

type_t * get_chunk_type()
{
    static type_t chunk_type = {
        .parameters = NULL,
        .alloc = &simple_alloc,
        .copy = &simple_copy,
        .free = &simple_free,
    };
    chunk_type.data_size = global_chunk_size * sizeof(double);
    return &chunk_type;
}

// ---

static char * double_str (object_t * obj)
{
    return rsprintf("%f", CAST_OBJECT(double, obj));
}

static type_t _double_type = {
    .parameters = NULL,
    .data_size = sizeof(double),
    .alloc = &simple_alloc,
    .copy = &simple_copy,
    .free = &simple_free,
    .str = &double_str,
};

type_t * double_type = &_double_type;

// -

static char * long_str (object_t * obj)
{
    return rsprintf("%ld", CAST_OBJECT(long, obj));
}

static type_t _long_type = {
    .parameters = NULL,
    .data_size = sizeof(long),
    .alloc = &simple_alloc,
    .copy = &simple_copy,
    .free = &simple_free,
    .str = &long_str,
};

type_t * long_type = &_long_type;
