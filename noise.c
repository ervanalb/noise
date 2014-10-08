#include <stdio.h>
#include "globals.h"
#include "block.h"
#include "wave.h"
#include "soundcard.h"

double global_frame_rate = 48000;
int global_chunk_size = 128;

error_t constant_frequency(node_t * node, output_pt * output)
{
	static double freq=440;
	*output = (output_pt*)(&freq);
	return SUCCESS;
}

/*
int handle_error(){
	fputs("Error caught!\n", stderr);
	fputs(global_error.message, stderr);
	fputc('\n', stderr);
	return global_error.code;
}
*/

void play(node_t* n, pull_fn_pt pull_f)
{
	soundcard_init();

	double* output;

	for(;;)
	{
		(*pull_f)(n, (output_pt*)(&output));
		soundcard_write(output);
	}

	soundcard_deinit();
}

