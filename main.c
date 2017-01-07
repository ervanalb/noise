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

coroutine void nz_mixer(const char * name, size_t n_pipes) {
    struct {
        int in_ch;
        nz_real volume;

        char in_name[32];
        char vol_name[32];
    } channels[n_pipes];
    for (size_t i = 0; i < n_pipes; i++) {

        channels[i].volume = 1.0;
        snprintf(channels[i].in_name, sizeof channels[i].in_name, "Mixer Input #%zu Channel", i);
        snprintf(channels[i].vol_name, sizeof channels[i].vol_name, "Mixer Input #%zu Volume", i);

        channels[i].in_ch = nz_param_channel(name, channels[i].in_name, NZ_READ);
        nz_param_real(name, channels[i].vol_name, 0.0, 1.0, &channels[i].volume);
    }

    int output = nz_param_channel(name, "Output Channel", NZ_WRITE);

    while (1) {
        nz_real * chunk = nz_challoc(output);
        memset(chunk, 0, sizeof(nz_chunk));
        for (size_t i = 0; i < n_pipes; i++) {
            const nz_real * c = nz_chrecv(channels[i].in_ch, NZ_CANSKIP);
            if (c == NULL)
                continue;

            for (size_t k = 0; k < NZ_CHUNK_SIZE; k++)
                chunk[k] += c[k] * channels[i].volume;
        }
        int rc = nz_chsend(output, chunk, 0);
        ASSERT(rc == 0);
    }
}

void start_noise(void) {
    go(nz_output_wav("WAV Output", "test.wav"));
    go(nz_output_portaudio("Live Output"));
    go(nz_mixer("Mixer", 2));
    //go(nz_synth("Synth"));
    //go(nz_synth("Synth2"));
    nz_block_loadlib("/home/zbanks/noise-dsock/blocks/synth.nzo");
    nz_block_start("synth", "Synthesizer", NULL);
}

int main(void) {
    start_noise();
    nz_param_ui();
}
