#ifndef __CORE_NTYPE_H__
#define __CORE_NTYPE_H__

#include <stddef.h>

#include "core/error.h"

// Stuff relating to types

struct nz_obj;
struct nz_type;

// Be careful creating your own instances of `struct nz_type`
// Make sure you alloc space for `type_parameters`
struct nz_type {
    // Size only refers to the 'internal' size, 
    // i.e. the size of the `obj.obj_data` field.
    size_t type_size;

    struct nz_obj * (*type_create)(const struct nz_type * type);
    void (*type_destroy)(struct nz_obj * obj);
    struct nz_obj * (*type_copy)(struct nz_obj * dst, const struct nz_obj * src);
    char * (*type_str)(const struct nz_obj * obj);

    char type_parameters[0];
};

// This should be treated like an opaque type
// (It's not for performance reasons)
struct nz_obj {
    const struct nz_type * obj_type;
    char obj_data[0];
};

// Methods on `struct nz_obj` objects
struct nz_obj * nz_obj_create(const struct nz_type * type);
void nz_obj_destroy(struct nz_obj ** obj);
struct nz_obj * nz_obj_copy(struct nz_obj * dst, const struct nz_obj * src);
char * nz_obj_str(const struct nz_obj * obj);

struct nz_obj * nz_obj_dup(const struct nz_obj * src);
struct nz_obj * nz_obj_swap(struct nz_obj ** store, const struct nz_obj * src);

static inline const struct nz_type * nz_obj_type(const struct nz_obj * obj) {
    return obj->obj_type;
}
//

struct nz_type * nz_type_create_simple(size_t size);
struct nz_type * nz_type_create_vector(size_t element_size);
// TODO Array type is untested, needs some more thought...
//struct nz_type * nz_type_create_array(size_t length, const struct nz_type * element_type);
// Deprecating tuple type for now... vector<struct nz_obj *> instead
//struct nz_type * nz_type_create_tuple(size_t length);

static inline int nz_type_compatible(const struct nz_type * ta, const struct nz_type * tb) {
    // This might change if the type system gets more complicated
    return ta == NULL || tb == NULL || ta == tb;
}

static inline int nz_obj_type_compatible(const struct nz_obj * oa, const struct nz_obj * ob) {
    return (oa != NULL && ob != NULL) && nz_type_compatible(nz_obj_type(oa), nz_obj_type(ob));
}

// These are exported because they may be useful in defining custom types
// They're suffixed with a _ to let you know you shouldn't call them directly.
struct nz_obj * nz_simple_create_(const struct nz_type * type);
void nz_simple_destroy_(struct nz_obj * obj);
struct nz_obj * nz_simple_copy_(struct nz_obj * dst, const struct nz_obj * src);

//TODO: rename set_size -> resize, get_size -> size
size_t nz_vector_set_size(struct nz_obj * obj, size_t new_size);
size_t nz_vector_get_size(const struct nz_obj * obj);
void * nz_vector_at(struct nz_obj * vec, size_t idx);
int nz_vector_push_back(struct nz_obj * vec, void * data);
void nz_vector_erase(struct nz_obj * vec, size_t idx);
size_t nz_vector_sizeofel(struct nz_obj * vec);

// We're abusing type punning here, but it's OK!
// `obj_data` will only be accessed through exactly one pointer type
// It would have been a `void *` but this way we save some indirection
#define NZ_CAST(type, obj) (*((type *)((union { char * c; type* x;})(char *)((obj)->obj_data)).x))

// This is just grody
//#define NZ_CAST_TUPLE(type, idx, obj) CAST_OBJECT(type, (&CAST_OBJECT(struct nz_obj *, (obj)))[(idx)])

//TODO: How do we provide a consistent way to do type resolution/lookup?

// Single-element array to make a pointer
// E.g. double_type is a (const struct nz_type *)
// As in `object_create(double_type)` not `object_create(&double_type)`
extern const struct nz_type nz_double_type[1];
extern const struct nz_type nz_long_type[1];
extern const struct nz_type nz_string_type[1];
extern struct nz_type nz_chunk_type[1];
extern struct nz_type * nz_sample_type;
extern struct nz_type * nz_note_vector_type;
extern struct nz_type * nz_object_vector_type;

// nb: there is no way to 'destroy' or 'free' types once created.

#endif
