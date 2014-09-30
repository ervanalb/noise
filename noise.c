#include <stdio.h>
#include "globals.h"
#include "node.h"
#include "wave.h"
#include "soundcard.h"
#include "error.h"

double global_frame_rate = 48000;
int global_chunk_size = 128;

void* constant_frequency(node_t* node)
{
	static double freq=440;
	return &freq;
}

int handle_error(){
    fputs("Error caught!\n", stderr);
    fputs(global_error.message, stderr);
    fputc('\n', stderr);
    return global_error.code;
}

int main()
{
	node_t wave;
	pull_fn_t ui_pulls[1] = {&constant_frequency};

    if(wave_new(&wave)) return handle_error();
	wave.input_pull = ui_pulls;
	wave.input=0;

	soundcard_init();

	double* output;

	for(;;)
	{
		if(wave_pull(&wave, (void **) &output)) return handle_error();
		soundcard_write(output);
	}

	if(wave_del(&wave)) return handle_error();
	soundcard_deinit();

	return 0;
}
