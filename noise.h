#pragma once

#include <libdill/libdill.h>

// Global settings

#define NZ_CHUNK_SIZE 128
#define NZ_SAMPLE_RATE 44100
typedef double nz_real;

// Channels

static inline int
nz_chmake() {
    return chmake(NZ_CHUNK_SIZE * sizeof(nz_real));
}

static inline int
nz_chsend(int ch, const nz_real * chunk) {
    return chsend(ch, chunk, NZ_CHUNK_SIZE * sizeof *chunk, -1);
}

static inline int
nz_chrecv(int ch, nz_real * chunk) {
    return chrecv(ch, chunk, NZ_CHUNK_SIZE * sizeof *chunk, -1);
}

static inline int
nz_chdone(int ch) {
    return chdone(ch);
}

// UI
struct nz_param;

struct nz_enum {
    int value;
    const char * name;
};

struct nz_param *
nz_param_enum(const char * name, const struct nz_enum * enums, int * param);

struct nz_param *
nz_param_real(const char * name, nz_real min, nz_real max, nz_real * param);

void
nz_param_destroy(struct nz_param * cookie);

coroutine void
nz_param_ui();
