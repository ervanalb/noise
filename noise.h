#ifndef __NOISE_H__
#define __NOISE_H__

#include <stddef.h>

// All symbols included in this file should be prefixed with `nz_`
// This is intended to be the library API

// ------------------------------------
// --------------- MISC ---------------
// ------------------------------------

typedef long nz_int;
typedef long long nz_long;

#ifdef NZ_REAL_FLOAT
typedef float nz_real;
#else
typedef double nz_real;
#endif

// ------------------------------------
// ---------- ERROR HANDLING ----------
// ------------------------------------

#define DECLARE_ERRORS \
    DECLARE_ERROR(NZ_NOT_IMPLEMENTED) \
    DECLARE_ERROR(NZ_NOT_ENOUGH_MEMORY) \
    DECLARE_ERROR(NZ_TYPE_NOT_FOUND) \
    DECLARE_ERROR(NZ_UNEXPECTED_ARGS) \
    DECLARE_ERROR(NZ_ARG_PARSE) \
    DECLARE_ERROR(NZ_BAD_PARG) \
    DECLARE_ERROR(NZ_BAD_KWARG) \
    DECLARE_ERROR(NZ_ARG_MISSING) \
    DECLARE_ERROR(NZ_ARG_VALUE) \
    DECLARE_ERROR(NZ_BLOCK_NOT_FOUND) \
    DECLARE_ERROR(NZ_NODE_NOT_FOUND) \
    DECLARE_ERROR(NZ_PORT_NOT_FOUND) \
    DECLARE_ERROR(NZ_PORT_ALREADY_CONNECTED) \
    DECLARE_ERROR(NZ_TYPE_MISMATCH) \
    DECLARE_ERROR(NZ_PORTS_NOT_CONNECTED) \
    DECLARE_ERROR(NZ_INTERNAL_ERROR) \
    DECLARE_ERROR(NZ_CANT_LOAD_LIBRARY)

// Errors are passed through this return code object:
#define DECLARE_ERROR(X) X ,
typedef enum {
    NZ_SUCCESS = 0,
    DECLARE_ERRORS
} nz_rc;
#undef DECLARE_ERROR

extern char * nz_error_string;
extern const char * nz_error_file;
extern int nz_error_line;

#define NZ_ERR_MSG(STR) {nz_error_file = __FILE__; nz_error_line = __LINE__; nz_error_string = STR;}
#define NZ_ERR() NZ_ERR_MSG(NULL)
#define NZ_RETURN_ERR(ERR) {NZ_ERR(); return (ERR);}
#define NZ_RETURN_ERR_MSG(ERR, STR) {NZ_ERR_MSG(STR); return (ERR);}

const char * nz_error_rc_str(nz_rc rc);
void nz_error_string_free();

// ------------------------------------
// ------------- TYPES ----------------
// ------------------------------------

typedef void nz_obj;
typedef void nz_type;
struct nz_typeclass;

// ------------------------------------
// ------------- BLOCKS ---------------
// ------------------------------------

struct nz_blockclass;
typedef void nz_block_state;
struct nz_block;
typedef nz_obj * nz_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p);

nz_obj * nz_null_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p);

struct nz_block {
    nz_block_state *  block_state_p;
    nz_pull_fn **     block_upstream_pull_fn_p_array;
    struct nz_block * block_upstream_block_array;
    size_t *          block_upstream_output_index_array;
};

struct nz_port_info {
    char *                      block_port_name;
    const struct nz_typeclass * block_port_typeclass_p;
    nz_type *                   block_port_type_p;
};

struct nz_block_info {
    size_t                block_n_inputs;
    struct nz_port_info * block_input_port_array;

    size_t                block_n_outputs;
    struct nz_port_info * block_output_port_array;
    nz_pull_fn **         block_pull_fns;
};

// ------------------------------------
// ------------ CONTEXT ---------------
// ------------------------------------

struct nz_context;
struct nz_lib;

nz_rc nz_context_create(struct nz_context ** context_pp);
void nz_context_destroy(struct nz_context * context_p);

nz_rc nz_context_register_blockclass(struct nz_context * context_p, struct nz_blockclass const * blockclass_p);
nz_rc nz_context_register_typeclass(struct nz_context * context_p, struct nz_typeclass const * typeclass_p);

nz_rc nz_block_list(struct nz_context * context_p, char const *** blockclass_string_array_p);
nz_rc nz_type_list(struct nz_context * context_p, char const *** typeclass_string_array_p);

nz_rc nz_context_load_lib(struct nz_context * context_p, const char * lib_name, struct nz_lib ** lib_handle);
void nz_context_unload_lib(struct nz_context * context_p, struct nz_lib * lib_handle);

// ------------------------------------
// -------------- GRAPH ---------------
// ------------------------------------

struct nz_graph;

nz_rc nz_graph_create(const struct nz_context * context_p, struct nz_graph ** graph_pp);
void nz_graph_destroy(struct nz_graph * graph_p);

nz_rc nz_graph_add_block(struct nz_graph * graph_p, const char * id, const char * block);
nz_rc nz_graph_del_block(struct nz_graph * graph_p, const char * id);

nz_rc nz_graph_connect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream);
nz_rc nz_graph_disconnect(struct nz_graph * graph_p, const char * id_upstream, const char * port_upstream, const char * id_downstream, const char * port_downstream);

nz_rc nz_graph_block_info(struct nz_graph * graph_p, const char * id, struct nz_block_info ** info_pp);
nz_rc nz_graph_block_handle(struct nz_graph * graph_p, const char * id, struct nz_block ** block_pp);

// ------------------------------------

extern const size_t nz_chunk_size;
extern const size_t nz_frame_rate;

#endif
