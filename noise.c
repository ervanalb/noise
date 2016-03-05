#include <math.h>
#include <stdlib.h>

#include <sndfile.h>
#include "libdill.h"

#include "noise.h"

coroutine int nz_saw(int ch_freq, int ch_output) {
    nz_real phase = 0;
    while (1) {
        nz_chunk buffer;
        nz_real freq;
        int rc = chrecv(ch_freq, &freq, sizeof freq, -1);
        if (rc != 0) break;
        
        nz_real dphase = freq / NZ_FRAME_RATE;
        for (size_t i = 0; i < NZ_CHUNK_SIZE; i++) {
            buffer[i] = phase * 2 - 1.;
            phase = fmod(phase + dphase, 1.0);
        }

        rc = chsend(ch_output, &buffer, sizeof buffer, -1);
        if (rc != 0) break;
    }
    return (errno == ECANCELED || errno == EPIPE) ? 0 : -1;
}

coroutine int nz_wav_record(const char * filename, int ch_input) {
    SF_INFO fdata = {
        .samplerate = NZ_FRAME_RATE,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16,
    };
    SNDFILE * file = sf_open(filename, SFM_WRITE, &fdata);
    if (file == NULL) {
        printf("libsndfile error: %s", sf_strerror(file));
        errno = EINVAL;
        return -1;
    }

    while (1) {
        nz_chunk buffer;
        int rc = chrecv(ch_input, &buffer, sizeof buffer, -1);
        if (rc != 0) break;

        sf_write_float(file, buffer, NZ_CHUNK_SIZE);
    }

    sf_write_sync(file);
    sf_close(file);

    return (errno == ECANCELED || errno == EPIPE) ? 0 : -1;
}
