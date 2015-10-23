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
    void (*type_destroy)     (nz_type_p type_p);
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

extern const struct nz_typeclass nz_int_type;
extern const struct nz_typeclass nz_long_type;
extern const struct nz_typeclass nz_real_type;
extern const struct nz_typeclass nz_string_type;
extern const struct nz_typeclass nz_chunk_type;
extern const struct nz_typeclass nz_sample_type;
extern const struct nz_typeclass nz_array_type;
extern const struct nz_typeclass nz_vector_type;
//extern const struct nz_typeclass nz_note_vector_type;

nz_rc nz_array_type_init(nz_type_p type_p, int n, const struct nz_typeclass * typeclass_p, nz_type_p type_p);
nz_rc nz_vector_type_init(nz_type_p type_p, const struct nz_typeclass * typeclass_p, nz_type_p type_p);

#endif
