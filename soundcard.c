#include <stdlib.h>
#include <string.h>
#include <portaudio.h>
#include "globals.h"
#include "block.h"
#include "typefns.h"
#include "blockdef.h"
#include "util.h"

size_t global_chunk_size = 128;
double global_frame_rate = 48000;

PaStreamParameters outputParameters;
PaError err;
PaStream* stream;
float* buffer;
static node_t * sc_node;

static int soundcard_api_init()
{
	/* -- initialize PortAudio -- */
	err = Pa_Terminate();
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

static int soundcard_api_write(double* chunk)
{
    //memcpy(buffer, chunk, global_chunk_size * sizeof(double));
    for (size_t i = 0; i < global_chunk_size; i++) {
        buffer[i] = (float) chunk[i];
    }
	err = Pa_WriteStream( stream, buffer, global_chunk_size);
	if( err ) return 1;
	return 0;
}

static int soundcard_api_deinit()
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

// --- Block methods ---

void soundcard_destroy(node_t * node)
{
    soundcard_api_deinit();
    node_destroy_generic(node);
    sc_node = NULL;
}

// Public

node_t * soundcard_get()
{
    if (sc_node) return sc_node;

    type_t * chunk_type = get_chunk_type();
    sc_node = node_alloc(1, 0, NULL);
    sc_node->name = strdup("Soundcard");
    sc_node->destroy = &soundcard_destroy;

    // Define inputs
    sc_node->inputs[0] = (struct node_input) {
        .type = chunk_type,
        .name = strdup("chunks"),
    };
    
    // Setup
#ifndef FAKESOUND
    soundcard_api_init();
#endif
    (void) soundcard_api_init;
    (void) soundcard_api_write;

    return sc_node;
}

void soundcard_run()
{
    if (!sc_node) {
        printf("Soundcard not initialized!\n");
        return;
    }

#ifndef FAKESOUND
    int iters = 10000;
#else
    int iters = 10;
#endif
    //while (1) {
    for (int i = 0; i < iters; i++) {
        object_t * chunk = NULL;
        node_pull(sc_node, 0, &chunk);

        if (chunk == NULL) return;

#ifndef FAKESOUND
        if (soundcard_api_write(&CAST_OBJECT(double, chunk))) return;
#endif
    }
}
