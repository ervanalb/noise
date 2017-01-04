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

// UI
struct nz_param;

struct nz_enum {
    int value;
    const char * name;
};

struct nz_param *
nz_param_enum(const char * parent, const char * name, const struct nz_enum * enums, int * param);

struct nz_param *
nz_param_real(const char * parent, const char * name, nz_real min, nz_real max, nz_real * param);

struct nz_param *
nz_param_channel(const char * parent, const char * name, int param);

void
nz_param_destroy(struct nz_param * cookie);

coroutine void
nz_param_ui();
