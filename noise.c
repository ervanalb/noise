#include "globals.h"
#include "node.h"
#include "wave.h"
#include "soundcard.h"

double global_frame_rate = 48000;
int global_chunk_size = 128;

void* constant_frequency(node_t* node)
{
	static double freq=440;
	return &freq;
}

int main()
{
	node_t wave;
	pull_fn_t ui_pulls[1] = {&constant_frequency};

    wave_new(&wave);
	wave.input_pull = ui_pulls;
	wave.input=0;

	soundcard_init();

	double* output;

	for(;;)
	{
		wave_pull(&wave, (void **) &output);
		soundcard_write(output);
	}

	wave_del(&wave);
	soundcard_deinit();

	return 0;
}
