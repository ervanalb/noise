#include <stdlib.h>

#include "noise.h"
#include "core/util.h"
#include "core/block.h"
#include "core/error.h"
#include "types/ntypes.h"
#include "core/argparse.h"

#include <portaudio.h>

struct pa_block_state {
    nz_real * chunk_p;
    float * buffer_p;
    PaStream * stream;
};

nz_rc pa_init() {
    PaError err = Pa_Initialize();
    if(err != paNoError) NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(Pa_GetErrorText(err)));
    return NZ_SUCCESS;
}

void pa_term() {
    Pa_Terminate();
}

nz_rc pa_start(struct nz_block * block_p) {
    struct nz_block self = *block_p;
    struct pa_block_state * state_p = (struct pa_block_state *)(self.block_state_p);

    PaError err = Pa_StartStream(state_p->stream);
    if(err != paNoError) NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(Pa_GetErrorText(err)));

    while(NZ_PULL(self, 0, state_p->chunk_p) != NULL) {
        for(size_t i = 0; i < nz_chunk_size; i++) {
            state_p->buffer_p[i] = state_p->chunk_p[i];
        }
        err = Pa_WriteStream(state_p->stream, state_p->buffer_p, nz_chunk_size);
        if(err != paNoError) NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(Pa_GetErrorText(err)));
    }

    err = Pa_StopStream(state_p->stream);
    if(err != paNoError) NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(Pa_GetErrorText(err)));

    return NZ_SUCCESS;
}

static nz_rc pa_block_create_args(PaDeviceIndex device, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct pa_block_state * state_p = calloc(1, sizeof(struct pa_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    state_p->chunk_p = calloc(nz_chunk_size, sizeof(nz_real));

    if(state_p->chunk_p == NULL) {
        free(state_p);
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    state_p->buffer_p = calloc(nz_chunk_size, sizeof(float));

    if(state_p->buffer_p == NULL) {
        free(state_p->chunk_p);
        free(state_p);
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    PaStreamParameters parameters;
    parameters.device = device;
    parameters.channelCount = 1;
    parameters.sampleFormat = paFloat32;
    parameters.suggestedLatency = 0;
    parameters.hostApiSpecificStreamInfo = 0;

    PaError err = Pa_OpenStream(&state_p->stream,
                                0,
                                &parameters,
                                nz_frame_rate,
                                nz_chunk_size,
                                0,
                                NULL,
                                NULL);

    if(err != paNoError) {
        free(state_p->chunk_p);
        free(state_p->buffer_p);
        free(state_p);
        NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(Pa_GetErrorText(err)));
    }

    nz_rc rc;
    if((rc = block_info_set_n_io(info_p, 1, 0)) != NZ_SUCCESS ||
       (rc = block_info_set_input(info_p, 0, strdup("in"), &nz_chunk_typeclass, NULL)) != NZ_SUCCESS) {
        block_info_term(info_p);
        Pa_CloseStream(state_p->stream);
        free(state_p->chunk_p);
        free(state_p->buffer_p);
        free(state_p);
        return rc;
    }

    *(struct pa_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc pa_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("int output_device", string, args);
    if(rc != NZ_SUCCESS) return rc;

    PaDeviceIndex output_device;
    if(args[0] == NULL) {
        output_device = Pa_GetDefaultOutputDevice();
    } else {
        output_device = *(long *)args[0];
        free(args[0]);
    }

    return pa_block_create_args(output_device, state_pp, info_p);
}

void pa_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct pa_block_state * pa_block_state_p = (struct pa_block_state *)state_p;

    block_info_term(info_p);
    Pa_CloseStream(&pa_block_state_p->stream);
    free(pa_block_state_p->chunk_p);
    free(pa_block_state_p->buffer_p);
    free(pa_block_state_p);
}

DECLARE_BLOCKCLASS(pa)
