#pragma once

#include <libdill/libdill.h>

// Global settings

#define NZ_CHUNK_SIZE 128
#define NZ_SAMPLE_RATE 44100
#define NZ_REAL_TYPE double

typedef NZ_REAL_TYPE nz_real;
typedef nz_real nz_chunk[NZ_CHUNK_SIZE];

// Channels

#define NZ_CH_NORETRY (1 << 0)

int nz_chmake();
int nz_chsend(const int * ch, const nz_real * chunk, int flags);
int nz_chrecv(const int * ch, nz_real * chunk, int flags);
int nz_chdone(int ch);

// UI
struct nz_param;

struct nz_enum {
    int value;
    const char * name;
};

enum nz_direction {
    NZ_INPUT,
    NZ_OUTPUT,
    //NZ_BIDIR,
};

struct nz_param *
nz_param_enum(const char * parent, const char * name, const struct nz_enum * enums, int * param);

struct nz_param *
nz_param_real(const char * parent, const char * name, nz_real min, nz_real max, nz_real * param);

struct nz_param *
nz_param_channel(const char * parent, const char * name, enum nz_direction dir, int * param);

void
nz_param_destroy(struct nz_param * cookie);

coroutine void
nz_param_ui();
