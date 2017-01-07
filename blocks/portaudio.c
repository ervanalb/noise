#include "noise.h"
#include "log.h"
#include <sndfile.h>
#include <portaudio.h>

#define SF_WRITE_REAL1(X, Y) SF_WRITE_REAL2(X, Y)
#define SF_WRITE_REAL2(X, Y) X ## Y
#define SF_WRITE_REAL SF_WRITE_REAL1(sf_write_, NZ_REAL_TYPE)

coroutine void nz_output_wav(const char * name, const char * filename) {
    int input = nz_chmake(NZ_READ);
    nz_param_channel(name, "Input Channel", input);

    SF_INFO fdata = {
        .samplerate = NZ_SAMPLE_RATE,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16,
    };
    SNDFILE * file = sf_open(filename, SFM_WRITE, &fdata);
    if (file == NULL) {
        printf("libsndfile error: %s", sf_strerror(file));
        errno = EINVAL;
        return;
    }

    while (1) {
        uint64_t n = now();
        const nz_real * buffer = nz_chrecv(input, 0);
        if (buffer == NULL) break;

        SF_WRITE_REAL(file, buffer, NZ_CHUNK_SIZE);
        // FIXME: How do we keep things real-time?
        msleep(n + (1000 * NZ_CHUNK_SIZE) / NZ_SAMPLE_RATE);
    }

    sf_write_sync(file);
    sf_close(file);
}

coroutine void nz_output_portaudio(const char * name) {
    PaError err = Pa_Initialize();
    if(err != paNoError) FAIL("Pa_Initialize: %s", Pa_GetErrorText(err));

    PaStream * stream;
    err = Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, NZ_SAMPLE_RATE, NZ_CHUNK_SIZE, NULL, NULL);
    if(err != paNoError) FAIL("Pa_OpenStream: %s", Pa_GetErrorText(err));

    err = Pa_StartStream(stream);
    if(err != paNoError) FAIL("Pa_OpenStream: %s", Pa_GetErrorText(err));

    nz_real volume = 0.2;
    nz_param_real(name, "Volume", 0.0, 1.0, &volume);

    int input = nz_chmake(NZ_READ);
    nz_param_channel(name, "Input Channel", input);

    while (1) {
        float buf[NZ_CHUNK_SIZE];
        const nz_real * chunk = nz_chrecv(input, 0);
        if (chunk == NULL) break;

        for (size_t i = 0; i < NZ_CHUNK_SIZE; i++)
            buf[i] = chunk[i] * volume;

        err = Pa_WriteStream(stream, buf, NZ_CHUNK_SIZE);
        //if(err != paNoError) FAIL("Pa_WriteStream: %s", Pa_GetErrorText(err));
    }

    Pa_Terminate();
}
