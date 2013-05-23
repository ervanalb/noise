#include <stdio.h>
#include <math.h>
#include <portaudio.h>
#include "PortaudioBlock.h"

#define NUM_SECONDS   (100)
#define FRAMES_PER_BUFFER  (1024)

#ifndef PI
#define PI  (3.14159265)
#endif

PortaudioBlock::PortaudioBlock(int sample_rate){
    this->sample_rate = sample_rate;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int paCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
    PortaudioBlock* self = (PortaudioBlock*) userData;
    float *out = (float*)outputBuffer;
    float lv, rv;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;
    
    for( i=0; i<framesPerBuffer; i++ )
    {
        self->input_gen->get();
        if(self->input_gen->size() >= 2){
            if(self->input_gen->at(0) == 0){ // null ptr
                lv = 0.0;
            }else{
                lv = *(float *)(self->input_gen->at(0));
            }
            if(self->input_gen->at(0) == 0){ // null ptr
                rv = 0.0;
            }else{
                rv = *(float *)(self->input_gen->at(1));
            }
        }else if(self->input_gen->size() == 1){
            if(self->input_gen->at(0) == 0){ // null ptr
                lv = rv = 0.0;
            }else{
                lv = rv = *(float *)(self->input_gen->at(0));
            }
        }else{
            lv = 0.0;
            rv = 0.0;
        }

        *out++ = lv;
        *out++ = rv;
    }
    
    return paContinue;
}

/*
 * This routine is called by portaudio when playback is done.
 */
static void StreamFinished( void* userData )
{
   //Generator* outgen = (Generator*)userData;
   printf( "Stream Completed: \n");
}

/*******************************************************************/

void PortaudioBlock::start()
{
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;

    this->input_gen = inputs[0];

    
    printf("PortAudio: output SR = %d, BufSize = %d\n", this->sample_rate, FRAMES_PER_BUFFER);
    
    printf("devices: %u\n", Pa_GetDeviceCount());
    
    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
      fprintf(stderr,"Error: No default output device.\n");
      goto error;
    }
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              this->sample_rate,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              paCallback,
              this );
    if( err != paNoError ) goto error;

    err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
    if( err != paNoError ) goto error;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    
    // Why would we ever stop?!
    printf("Play for %d seconds.\n", NUM_SECONDS );
    Pa_Sleep( NUM_SECONDS * 1000 );

    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;

    Pa_Terminate();
    printf("Test finished.\n");

error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
}
