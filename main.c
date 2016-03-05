#include "noise.h"
#include <assert.h>

int main(void) {
    int ch_freq = channel(sizeof(nz_real), 0);
    int ch_buff = channel(sizeof(nz_chunk), 0);

    go(nz_saw(ch_freq, ch_buff));
    go(nz_wav_record("test.wav", ch_buff));

    for(nz_real f = 10; f < 2000; f *= 1.001) {
        chsend(ch_freq, &f, sizeof f, -1);
    }

    chdone(ch_freq);
    chdone(ch_buff);

    return 0;
}
