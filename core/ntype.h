#ifndef __CORE_NTYPE_H__
#define __CORE_NTYPE_H__

#include "core/error.h"

typedef void* nz_obj_p;
typedef void* nz_type_p;

struct nz_typeclass {
    // Static members
    const char* type_id;

    // Class methods
    nz_rc (*type_create)     (nz_type_p * type_p);
    void  (*type_destroy)    (nz_type_p type_p);
    nz_rc (*type_copy)       (nz_type_p dst_p, const nz_type_p src_p);
    int   (*type_is_equal)   (const nz_type_p type_p, const nz_type_p other_type_p);
    nz_rc (*type_create_obj) (const nz_type_p type_p, nz_obj_p * obj_pp);

    // Instance methods
    void  (*type_destroy_obj) (const nz_type_p type_p, nz_obj_p obj_p);
    nz_rc (*type_copy_obj)    (const nz_type_p type_p, nz_obj_p dst_p, const nz_obj_p nz_src_p);
    nz_rc (*type_str_obj)     (const nz_type_p type_p, const nz_obj_p obj_p, char ** string);
};

int nz_types_are_equal(const struct nz_typeclass * type_1_p, const nz_type_p type_1_data_p,
                       const struct nz_typeclass * type_2_p, const nz_type_p type_2_data_p);

// --

extern const struct nz_typeclass nz_int_typeclass;
extern const struct nz_typeclass nz_long_typeclass;
extern const struct nz_typeclass nz_real_typeclass;
extern const struct nz_typeclass nz_string_typeclass;
extern const struct nz_typeclass nz_chunk_typeclass;
extern const struct nz_typeclass nz_sample_typeclass;
extern const struct nz_typeclass nz_array_typeclass;
extern const struct nz_typeclass nz_vector_typeclass;
//extern const struct nz_typeclass nz_note_vector_type;

nz_rc nz_array_type_init(nz_type_p type_p, int n, const struct nz_typeclass * inner_typeclass_p, nz_type_p inner_type_p);
nz_rc nz_vector_type_init(nz_type_p type_p, const struct nz_typeclass * inner_typeclass_p, nz_type_p inner_type_p);

// -
// Method definition helper macros

#define GEN_SIMPLE_TYPE_FNS(NAME) \
static nz_rc NAME ## _type_create (nz_type_p * type_pp) {\
    *type_pp = 0; \
    return NZ_SUCCESS; \
}\
static void NAME ## _type_destroy (nz_type_p type_p) {}\
static int NAME ## _type_is_equal (nz_type_p type_p, nz_type_p other_type_p) { \
    return 1; \
} \
static nz_rc NAME ## _type_copy (nz_type_p dst_p, const nz_type_p src_p) { \
    return NZ_SUCCESS; \
}

#define GEN_STATIC_OBJ_FNS(NAME, SIZE) \
static nz_rc NAME ## _type_create_obj (nz_type_p type_p, nz_obj_p * obj_pp) { \
    *obj_pp = calloc(1, SIZE); \
    if(*obj_pp == 0) return NZ_NOMEM; \
    return NZ_SUCCESS; \
} \
static void NAME ## _type_destroy_obj (nz_type_p type_p, nz_obj_p obj_p) { \
    free(obj_p); \
} \

#define GEN_SHALLOW_COPY_FN(NAME, SIZE) \
static nz_rc NAME ## _type_copy_obj (nz_type_p type_p, nz_obj_p dst_p, const nz_obj_p src_p) { \
    memcpy(dst_p, src_p, SIZE); \
    return NZ_SUCCESS; \
}

#define GEN_PRIMITIVE_STRING_FNS(NAME, CTYPE, FORMAT_STR) \
static nz_rc NAME ## _type_str_obj (nz_type_p type_p, const nz_obj_p obj_p, char ** string) \
{ \
    *string = rsprintf(FORMAT_STR, *(CTYPE *)obj_p); \
    if(*string == 0) return NZ_NOMEM; \
    return NZ_SUCCESS; \
}

#define DECLARE_TYPECLASS(NAME) \
static const char NAME ## _type_id[] = #NAME; \
const struct nz_typeclass nz_ ## NAME ## _typeclass = { \
    .type_id = NAME ## _type_id, \
    .type_create = & NAME ## _type_create, \
    .type_destroy = & NAME ## _type_destroy, \
    .type_copy = & NAME ## _type_copy, \
    .type_is_equal = & NAME ## _type_is_equal, \
    .type_create_obj = & NAME ## _type_create_obj, \
    .type_destroy_obj = & NAME ## _type_destroy_obj, \
    .type_copy_obj = & NAME ## _type_copy_obj, \
    .type_str_obj = & NAME ## _type_str_obj, \
};

#define DECLARE_PRIMITIVE_TYPECLASS(NAME, CTYPE, FORMAT_STR) \
GEN_SIMPLE_TYPE_FNS(NAME) \
GEN_STATIC_OBJ_FNS(NAME, sizeof(CTYPE)) \
GEN_SHALLOW_COPY_FN(NAME, sizeof(CTYPE)) \
GEN_PRIMITIVE_STRING_FNS(NAME, CTYPE, FORMAT_STR) \
DECLARE_TYPECLASS(NAME) \



#endif
