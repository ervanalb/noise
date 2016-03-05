#ifndef __NOISE_H__
#define __NOISE_H__

#include "libdill.h"

#define NZ_CHUNK_SIZE 128
#define NZ_FRAME_RATE 44100

typedef float nz_real;
typedef nz_real nz_chunk[NZ_CHUNK_SIZE];

coroutine int nz_saw(int freq, int output);
coroutine int nz_wav_record(const char * filename, int input);

#endif
