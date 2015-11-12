#include <stdlib.h>

#include "noise.h"
#include "core/util.h"
#include "core/ntype.h"
#include "core/ntypes.h"
#include "core/block.h"
#include "core/error.h"

#include <portaudio.h>

struct pa_block_state {
    nz_real * chunk_p;
    float * buffer_p;
    PaStream * stream;
};

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
                                48000,
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
    if(string == NULL) {
        return pa_block_create_args(Pa_GetDefaultOutputDevice(), state_pp, info_p);
    }

    const char * pos = string;
    const char * start;
    size_t length;
    int end;
    nz_rc rc;

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    rc = nz_next_block_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * device_index_str = strndup(start, length);
    if(device_index_str == NULL) {
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    PaDeviceIndex device_index;
    if(sscanf(device_index_str, "%d%n", &device_index, &end) != 1 || end <= 0 || (size_t)end != length) {
        free(device_index_str);
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    }
    free(device_index_str);

    if(pos != NULL) {
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    }

    return pa_block_create_args(device_index, state_pp, info_p);
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
