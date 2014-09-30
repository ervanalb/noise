#include <stdlib.h>
#include <portaudio.h>
#include "globals.h"

PaStreamParameters outputParameters;
PaError err;
PaStream* stream;
float* buffer;

int soundcard_init()
{
	/* -- initialize PortAudio -- */
	err = Pa_Initialize();
	if( err != paNoError ) return 1;
	/* -- setup output -- */
	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	outputParameters.channelCount = 1;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = 0;
	outputParameters.hostApiSpecificStreamInfo = 0;
	/* -- setup stream -- */
	err = Pa_OpenStream(
		  &stream,
		  0,
		  &outputParameters,
		  global_frame_rate,
		  global_chunk_size,
		  0,
		  0, /* no callback, use blocking API */
		  0 ); /* no callback, so no callback userData */
	if( err != paNoError ) return 1;
	/* -- start stream -- */
	err = Pa_StartStream( stream );
	if( err != paNoError ) return 1;

	buffer=malloc(global_chunk_size*sizeof(float));

	return 0;
}

int soundcard_write(double* chunk)
{
	int i;
	for(i=0;i<global_chunk_size;i++) buffer[i]=chunk[i];
	err = Pa_WriteStream( stream, buffer, global_chunk_size);
	if( err ) return 1;
	return 0;
}

int soundcard_deinit()
{
	free(buffer);
	/* -- Now we stop the stream -- */
	err = Pa_StopStream( stream );
	if( err != paNoError ) return 1;
	/* -- don't forget to cleanup! -- */
	err = Pa_CloseStream( stream );
	if( err != paNoError ) return 1;
	Pa_Terminate();
	return 0;
}
