#include "globals.h"
#include "node.h"
#include "wave.h"

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

	wave.state = wave_new();
	wave.input_pull = ui_pulls;
	wave.input=0;

	double* output=(double*)wave_pull(&wave);

	int i;
	for(i=0;i<global_chunk_size;i++)
	{
		printf("%lf\n",output[i]);
	}

	wave_del(wave.state);

	return 0;
}
