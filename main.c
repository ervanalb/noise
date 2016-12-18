#include "noise.h"
#include "log.h"
#include "output.h"
#include <stdio.h>
#include <math.h>

enum loglevel loglevel;

coroutine void test_noise() {
    int x = 0;
    nz_real y = 0;

    const struct nz_enum x_enums[] = {
        { 0, "off"},
        { 1, "on"},
        { 2, "super"},
        { 0, 0 }
    };
    nz_param_enum("test", "x", x_enums, &x);
    nz_param_real("test", "y", 0, 1, &y);

    while (1) {
        msleep(now() + 1000);
        fprintf(stderr, "\rx=%d y=%f", x, y);
    }
}

coroutine void nz_synth(const char * name, int output_ch) {
    nz_real phase = 0;
    nz_real freq = 440.;
    nz_param_real(name, "Frequency", 32, 16000, &freq);

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

    nz_param_channel(name, "Output Channel", NZ_OUTPUT, &output_ch);

    while (1) {
        nz_chunk buffer;
        nz_real dphase = freq / NZ_SAMPLE_RATE;
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

        int rc = nz_chsend(&output_ch, buffer, 0);
        if (rc != 0) break;
    }
}

coroutine void nz_mixer(const char * name, int * in_chans, size_t n_chans, int out_ch) {
    struct {
        int in_ch;
        nz_real volume;

        char in_name[32];
        char vol_name[32];

        nz_chunk chunk;
    } channels[n_chans];
    for (size_t i = 0; i < n_chans; i++) {
        channels[i].in_ch = in_chans[i];
        channels[i].volume = 1.0;
        snprintf(channels[i].in_name, sizeof channels[i].in_name, "Mixer Input #%zu Channel", i);
        snprintf(channels[i].vol_name, sizeof channels[i].vol_name, "Mixer Input #%zu Volume", i);

        nz_param_channel(name, channels[i].in_name, NZ_INPUT, &channels[i].in_ch);
        nz_param_real(name, channels[i].vol_name, 0.0, 1.0, &channels[i].volume);
    }

    nz_param_channel(name, "Output Channel", NZ_OUTPUT, &out_ch);

    while (1) {
        nz_chunk chunk;
        memset(chunk, 0, sizeof chunk);
        for (size_t i = 0; i < n_chans; i++) {
            if (channels[i].in_ch < 0)
                continue;
            int rc = nz_chrecv(&channels[i].in_ch, channels[i].chunk, NZ_CH_NORETRY);
            if (rc < 0 && channels[i].in_ch == -1)
                continue;
            ASSERT(rc == 0);
            for (size_t k = 0; k < NZ_CHUNK_SIZE; k++)
                chunk[k] += channels[i].chunk[k] * channels[i].volume;
        }
        int rc = nz_chsend(&out_ch, chunk, 0);
        ASSERT(rc == 0);
    }
}

void start_noise(void) {
    int out_ch = nz_chmake();
    ASSERT(out_ch >= 0);

    int mixer_chs[2];
    mixer_chs[0] = nz_chmake();
    mixer_chs[1] = -1;

    go(nz_output_wav("WAV Output", "test.wav", -1));
    go(nz_output_portaudio("Live Output", out_ch));
    go(nz_mixer("Mixer", mixer_chs, 2, out_ch));
    go(nz_synth("Synth", mixer_chs[0]));
}

int main(void) {
    start_noise();
    nz_param_ui();
}
