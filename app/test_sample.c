#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sndfile.h>

#include "noise.h"
#include "debug.h"
#include "blocks/blocks.h"
#include "core/util.h"
#include "samples/samples.h"

const size_t nz_chunk_size = 128;
const double nz_frame_rate = 44100;

int write_sample(struct nz_obj * sample, const char * filename) {
    printf("Writing %lu frames to %s", nz_vector_get_size(sample), filename);

    SF_INFO fdata = {
        .frames = nz_vector_get_size(sample),
        .samplerate = nz_frame_rate,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16,
        .sections = 1,
        .seekable = 0,
    };

    SNDFILE * f = sf_open(filename, SFM_WRITE, &fdata);
    sf_write_double(f, NZ_CAST(double *, sample), nz_vector_get_size(sample));
    sf_write_sync(f);
    sf_close(f);
    return 0;
}

int main(void) {
    nz_obj_create(nz_double_type);

    struct nz_obj * sample = synth_drum(2 * nz_frame_rate); // 2 seconds
    if (sample == NULL) return (printf("no sample\n"), -1);
    
    write_sample(sample, "drum.wav");

    return 0;
}
