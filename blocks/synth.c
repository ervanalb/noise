#include "noise.h"
#include "log.h"
#include <stdio.h>
#include <math.h>

coroutine void synth(const char * name, const void * args) {
    (void) (args);

    nz_real phase = 0;
    nz_real freq = 440.;
    nz_param_real(name, "Frequency", 32, 16000, &freq);
    nz_param_real(name, "Phase", 0, 1, &phase);

    enum {
        SAW, SQUARE, SINE
    } wave_type;
    const struct nz_enum wave_enums[] = {
        { SAW, "Saw"},
        { SQUARE, "Square"},
        { SINE, "Sine"},
        { 0, 0 }
    };
    nz_param_enum(name, "Wave Type", wave_enums, (int *) &wave_type);

    int output = nz_param_channel(name, "Output Channel", NZ_WRITE);

    while (1) {
        nz_real * buffer = nz_challoc(output);
        if (buffer == NULL) break;
        nz_real dphase = freq * 1.0 / NZ_SAMPLE_RATE;
        for (size_t i = 0; i < NZ_CHUNK_SIZE; i++) {
            switch (wave_type) {
            case SAW:
                buffer[i] = phase * 2 - 1.;
                break;
            case SQUARE:
                buffer[i] = (phase > 0.5) ? 1 : -1;
                break;
            case SINE:
                buffer[i] = sinf(phase * 2 * M_PI);
                break;
            default:
                buffer[i] = 0;
                break;
            }
            phase = fmod(phase + dphase, 1.0);
        }

        int rc = nz_chsend(output, buffer, 0);
        if (rc != 0) break;
    }
    INFO("synth dying...");
}

NZ_BLOCKLIB_INIT(synth);
