#include <stdlib.h>
#include <string.h>
#include <portaudio.h>

#include "noise.h"
#include "blocks/io/blocks.h"

PaStreamParameters outputParameters;
PaError err;
PaStream* stream;
float* buffer;
static struct nz_node sc_node[1];
int sc_init = 0;

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
		  nz_frame_rate,
		  nz_chunk_size,
		  0,
		  0, /* no callback, use blocking API */
		  0 ); /* no callback, so no callback userData */
	if( err != paNoError ) return 1;
	/* -- start stream -- */
	err = Pa_StartStream( stream );
	if( err != paNoError ) return 1;

	buffer=malloc(nz_chunk_size*sizeof(float));

	return 0;
}

static int soundcard_api_write(double* chunk)
{
    //memcpy(buffer, chunk, nz_chunk_size * sizeof(double));
    for (size_t i = 0; i < nz_chunk_size; i++) {
        buffer[i] = (float) chunk[i];
    }
	err = Pa_WriteStream( stream, buffer, nz_chunk_size);
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

void soundcard_destroy(struct nz_node * node) {
    soundcard_api_deinit();
    nz_node_term_generic(node);
    sc_init = 0;
}

// Public

struct nz_node * nz_soundcard_get() {
    int rc = nz_node_alloc_ports(sc_node, 1, 0);
    if (rc != 0) return NULL;

    sc_node->node_term = &soundcard_destroy;
    sc_node->node_name = strdup("Soundcard");

    // Define inputs
    sc_node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_chunk_type,
        .inport_name = strdup("chunks"),
    };
    
    // Setup
#ifndef FAKESOUND
    soundcard_api_init();
#endif
    (void) soundcard_api_init;
    (void) soundcard_api_write;

    sc_init = 1;

    return sc_node;
}

void nz_soundcard_run() {
    if (!sc_init) {
        printf("Soundcard not initialized!\n");
        return;
    }

#ifndef FAKESOUND
    int iters = 2500;
#else
    int iters = 10;
#endif
    //while (1) {
    for (int i = 0; i < iters; i++) {
        nz_obj_p chunk = NZ_NODE_PULL(sc_node, 0);

        if (chunk == NULL) return;

#ifndef FAKESOUND
        if (soundcard_api_write(&*(double*)chunk)) {
            printf("pa error\n");
        }
#endif
    }
}
