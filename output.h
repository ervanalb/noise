#include "noise.h"

coroutine void nz_output_wav(const char * name, const char * wav_filename, int ch_input);
coroutine void nz_output_portaudio(const char * name, int ch_input);
