#include <stdlib.h>
#include <portaudio.h>
#include "globals.h"
#include "block.h"
#include "typefns.h"
#include "blockdef.h"

PaStreamParameters outputParameters;
PaError err;
PaStream* stream;
float* buffer;
node_t * sc_node;

static int soundcard_api_init()
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

static int soundcard_api_write(double* chunk)
{
	int i;
	for(i=0;i<global_chunk_size;i++) buffer[i]=chunk[i];
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
    node_t * sc_node = node_alloc(1, 0, NULL);
    sc_node->name = strdup("Soundcard");
    sc_node->destroy = &soundcard_destroy;

    // Define inputs
    sc_node->inputs[0] = (struct node_input) {
        .type = chunk_type,
        .name = strdup("chunks"),
    };
    
    // Setup
    soundcard_api_init();

    return sc_node;
}

void soundcard_run()
{
    if (!sc_node) printf("Soundcard not initialized!\n");

    object_t * chunk = NULL;
    pull(sc_node, 1, &chunk);

    if (chunk == NULL) return;

    soundcard_api_write(CAST_OBJECT(double *, chunk));

}
