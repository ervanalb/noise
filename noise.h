#pragma once

#include <libdill/libdill.h>

// Global settings

#define NZ_CHUNK_SIZE 128
#define NZ_SAMPLE_RATE 44100
#define NZ_REAL_TYPE double

typedef NZ_REAL_TYPE nz_real;
typedef nz_real nz_chunk[NZ_CHUNK_SIZE];

// Channels

enum nz_dir {
    NZ_WRITE,
    NZ_READ,
};
#define NZ_NONBLOCK (1 << 0)
#define NZ_CANSKIP  (1 << 1)

int nz_chmake(enum nz_dir dir);
nz_real * nz_challoc(int ch);
int nz_chsend(int ch, const nz_real * chunk, int flags);
const nz_real * nz_chrecv(int ch, int flags);
int nz_chjoin(int left, int right);
int nz_chstate(int ch);
//int nz_choose(...);

// Blocks
#define CONCAT(x,y) CONCAT2(x,y)
#define CONCAT2(x,y) x##y
#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

typedef void (*nz_block_fn_t)(const char *, const void *);
void nz_blocklib_init(const char * filename, const char * fn_name, nz_block_fn_t * fn);

int nz_block_start(const char * fn_name, const char * name, const void * args);
int nz_block_reload(const char * fn_name);
int nz_block_loadlib(const char * filename);
struct nz_block * nz_block_get(const char * name);

#define NZ_BLOCKLIB_VAR _nz_block_fn
#define NZ_BLOCKLIB_VAR_STR STRINGIFY(NZ_BLOCKLIB_VAR)
#define NZ_BLOCKLIB_INIT(fn) \
    const char * NZ_BLOCKLIB_VAR = STRINGIFY(fn); \
    static const nz_block_fn_t _nz_block_typetest = &fn; 
/*
#define NZ_BLOCKLIB_INIT(fn) \
__attribute__((constructor)) static void CONCAT(nz_blocklib_init_,fn)(void) { \
    nz_blocklib_init(__FILE__ ".nzo", STRINGIFY(fn), fn); \
}
*/

// UI
struct nz_param;

struct nz_enum {
    int value;
    const char * name;
};

int
nz_param_block(const char * parent, const char * libname);

int
nz_param_enum(const char * parent, const char * name, const struct nz_enum * enums, int * param);

int
nz_param_real(const char * parent, const char * name, nz_real min, nz_real max, nz_real * param);

int
nz_param_channel(const char * parent, const char * name, enum nz_dir dir);

//int nz_param_opaque(const char * parent, const char * name, void * param, size_t size);

void
nz_param_destroy(struct nz_param * cookie);

coroutine void
nz_param_ui();
