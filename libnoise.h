#ifndef __NZ_MOD_H__
#define __NZ_MOD_H__

// This file is used to write new blocks and types

#include "noise.h"

// ------------------------------------
// -------- ARGUMENT PARSING  ---------
// ------------------------------------

typedef void nz_arg;

// Parse args manually
nz_rc next_arg(const char * string, const char ** pos,
               const char ** key_start, size_t * key_length,
               const char ** value_start, size_t * value_length);

// Parse args according to a format string
nz_rc arg_parse(const char * fmt, const char * args, nz_arg ** arg_p_array);

// ------------------------------------
// ------------- TYPES ----------------
// ------------------------------------

struct nz_typeclass {
    // Static members
    const char * type_id;
    nz_rc (*type_create)      (const struct nz_context * context_p, nz_type ** type_pp, const char * string);

    // Class methods
    void  (*type_destroy)     (nz_type * type_p);
    int   (*type_is_equal)    (const nz_type * type_p, const nz_type * other_type_p);
    nz_rc (*type_str)         (const nz_type * type_p, char ** string);
    nz_rc (*type_create_obj)  (const nz_type * type_p, nz_obj ** obj_pp);

    // Instance methods
    nz_rc (*type_init_obj)    (const nz_type * type_p, nz_obj * obj_p, const char * string);
    nz_rc (*type_copy_obj)    (const nz_type * type_p, nz_obj * dst_p, const nz_obj * src_p);
    void  (*type_destroy_obj) (const nz_type * type_p, nz_obj * obj_p);
    nz_rc (*type_str_obj)     (const nz_type * type_p, const nz_obj * obj_p, char ** string);
};

int nz_types_are_equal(const struct nz_typeclass * typeclass_p,       const nz_type * type_p,
                       const struct nz_typeclass * other_typeclass_p, const nz_type * other_type_p);

#define NZ_NULL_STR "NULL"

#define NZ_GEN_SIMPLE_TYPE_FNS(NAME) \
static nz_rc NAME ## _type_create (const struct nz_context * context_p, nz_type ** type_pp, const char * string) {\
    nz_rc result = arg_parse(NULL, string, NULL); \
    if(result != NZ_SUCCESS) return result; \
    *type_pp = NULL; \
    return NZ_SUCCESS; \
}\
static void NAME ## _type_destroy (nz_type * type_p) {}\
static int NAME ## _type_is_equal (const nz_type * type_p, const nz_type * other_type_p) { \
    return 1; \
} \
static nz_rc NAME ## _type_str (const nz_type * type_p, char ** string) {\
    *string = strdup(NAME ## _type_id);\
    if(*string == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY); \
    return NZ_SUCCESS;\
}

#define NZ_GEN_STATIC_OBJ_FNS(NAME, SIZE) \
static nz_rc NAME ## _type_create_obj (const nz_type * type_p, nz_obj * * obj_pp) { \
    *obj_pp = calloc(1, SIZE); \
    if(*obj_pp == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY); \
    return NZ_SUCCESS; \
} \
static void NAME ## _type_destroy_obj (const nz_type * type_p, nz_obj * obj_p) { \
    free(obj_p); \
} \

#define NZ_GEN_SHALLOW_COPY_FN(NAME, SIZE) \
static nz_rc NAME ## _type_copy_obj (const nz_type * type_p, nz_obj * dst_p, const nz_obj * src_p) { \
    memcpy(dst_p, src_p, SIZE); \
    return NZ_SUCCESS; \
}

#define NZ_GEN_PRIMITIVE_OBJ_STRING_FNS(NAME, CTYPE, FORMAT_STR) \
static nz_rc NAME ## _type_init_obj (const nz_type * type_p, nz_obj * obj_p, const char * string) { \
    CTYPE a; \
    int n; \
    int result = sscanf(string, FORMAT_STR "%n", &a, &n); \
    if(result != 1 || n < 0 || string[n] != '\0') NZ_RETURN_ERR_MSG(NZ_ARG_PARSE, strdup(string)); \
    *(CTYPE *)obj_p = a; \
    return NZ_SUCCESS; \
} \
static nz_rc NAME ## _type_str_obj (const nz_type * type_p, const nz_obj * obj_p, char ** string) { \
    *string = rsprintf(FORMAT_STR, *(CTYPE *)obj_p); \
    if(*string == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY); \
    return NZ_SUCCESS; \
}

#define NZ_DECLARE_TYPE_ID(NAME) \
static const char NAME ## _type_id[] = #NAME; \

#define NZ_DECLARE_TYPECLASS(NAME) \
const struct nz_typeclass nz_ ## NAME ## _typeclass = { \
    .type_id = NAME ## _type_id, \
    .type_create = & NAME ## _type_create, \
    .type_destroy = & NAME ## _type_destroy, \
    .type_is_equal = & NAME ## _type_is_equal, \
    .type_str = & NAME ## _type_str, \
    .type_create_obj = & NAME ## _type_create_obj, \
    .type_init_obj = & NAME ## _type_init_obj, \
    .type_destroy_obj = & NAME ## _type_destroy_obj, \
    .type_copy_obj = & NAME ## _type_copy_obj, \
    .type_str_obj = & NAME ## _type_str_obj, \
};

#define NZ_DECLARE_PRIMITIVE_TYPECLASS(NAME, CTYPE, FORMAT_STR) \
NZ_DECLARE_TYPE_ID(NAME) \
NZ_GEN_SIMPLE_TYPE_FNS(NAME) \
NZ_GEN_STATIC_OBJ_FNS(NAME, sizeof(CTYPE)) \
NZ_GEN_SHALLOW_COPY_FN(NAME, sizeof(CTYPE)) \
NZ_GEN_PRIMITIVE_OBJ_STRING_FNS(NAME, CTYPE, FORMAT_STR) \
NZ_DECLARE_TYPECLASS(NAME) \

// ------------------------------------
// ------------- BLOCKS ---------------
// ------------------------------------

struct nz_blockclass {
    const char * block_id;
    nz_rc (*block_create) (const struct nz_context * context, const char * string, nz_block_state ** state, struct nz_block_info * info);
    void (*block_destroy) (nz_block_state * state, struct nz_block_info * info);
};

#define NZ_PULL(SELF,INPUT,OBJ_P) ((SELF).block_upstream_pull_fn_p_array[(INPUT)]((SELF).block_upstream_block_array[(INPUT)], (SELF).block_upstream_output_index_array[(INPUT)], (OBJ_P)))

#define NZ_DECLARE_BLOCKCLASS(NAME) \
static const char NAME ## _block_id[] = #NAME; \
const struct nz_blockclass nz_ ## NAME ## _blockclass = { \
    .block_id = NAME ## _block_id, \
    .block_create = & NAME ## _block_create, \
    .block_destroy = & NAME ## _block_destroy, \
};

// --
// block_info helper functions

void nz_block_info_term(struct nz_block_info * info_p);
nz_rc nz_block_info_set_n_io(struct nz_block_info * info_p, size_t n_inputs, size_t n_outputs);
nz_rc nz_block_info_set_input(struct nz_block_info * info_p, size_t input_index, char * name, const struct nz_typeclass * typeclass_p, nz_type * type_p);
nz_rc nz_block_info_set_output(struct nz_block_info * info_p, size_t input_index, char * name, const struct nz_typeclass * typeclass_p, nz_type * type_p, nz_pull_fn * pull_fn_p);

// ------------------------------------
// -------------- UTIL ----------------
// ------------------------------------

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

struct strbuf {
    size_t len;
    size_t capacity;
};

char * strbuf_alloc  (struct strbuf * buf);
char * strbuf_clear  (struct strbuf * buf, char * str);
char * strbuf_resize (struct strbuf * buf, char * str, size_t capacity);
char * strbuf_printf (struct strbuf * buf, char * str, const char * fmt, ...) __attribute__ ((format (printf, 3, 4)));
char * strbuf_putc   (struct strbuf * buf, char * str, char c);

#ifndef __USE_BSD
char * strdup(const char * s);
char * strndup(const char * s, size_t maxlen);
#endif

int asprintf(char ** buf, const char * fmt, ...) __attribute__ ((format (printf, 2, 3)));
char * rsprintf(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif
