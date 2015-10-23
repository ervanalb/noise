#ifndef __CORE_NTYPE_H__
#define __CORE_NTYPE_H__

#include "core/error.h"

typedef void* nz_obj_p;
typedef void* nz_type_data_p;

struct nz_type {
    // Static members
    const char* type_id;

    // Class methods
    int   (*type_is_equal)   (nz_type_data_p type_data_p, nz_type_data_p other_type_data_p);
    nz_rc (*type_create_obj) (nz_type_data_p type_data_p, nz_obj_p * obj_pp);

    // Instance methods
    void  (*type_destroy_obj) (nz_type_data_p type_data_p, nz_obj_p obj_p);
    nz_rc (*type_copy_obj)    (nz_type_data_p type_data_p, nz_obj_p dst_p, const nz_obj_p nz_src_p);
    nz_rc (*type_str_obj)     (nz_type_data_p type_data_p, const nz_obj_p obj, char ** string);
};

int nz_types_are_equal(const struct nz_type * type_1_p, nz_type_data_p type_1_data_p,
                       const struct nz_type * type_2_p, nz_type_data_p type_2_data_p);

// --

extern const struct nz_type nz_int_type;
extern const struct nz_type nz_long_type;
extern const struct nz_type nz_double_type;
extern const struct nz_type nz_string_type;
extern const struct nz_type nz_chunk_type;
extern const struct nz_type nz_sample_type;
extern const struct nz_type nz_array_type;
extern const struct nz_type nz_vector_type;
//extern const struct nz_type nz_note_vector_type;

nz_rc nz_type_create_int    (nz_type_data_p * type_data_pp);
nz_rc nz_type_create_long   (nz_type_data_p * type_data_pp);
nz_rc nz_type_create_double (nz_type_data_p * type_data_pp);
nz_rc nz_type_create_string (nz_type_data_p * type_data_pp);
nz_rc nz_type_create_chunk  (nz_type_data_p * type_data_pp);
nz_rc nz_type_create_sample (nz_type_data_p * type_data_pp);
nz_rc nz_type_create_array  (nz_type_data_p * type_data_pp, int n, const struct nz_type * type_p, nz_type_data_p type_data_p);
nz_rc nz_type_create_vector (nz_type_data_p * type_data_pp, const struct nz_type * type_p, nz_type_data_p type_data_p);

#endif

