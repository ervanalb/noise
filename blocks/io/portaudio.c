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
    PaStream * stream;
};

nz_rc pa_pull(struct nz_block * block_p) {
    struct nz_block self = *block_p;
    struct pa_block_state * state_p = (struct pa_block_state *)(self.block_state_p);

    nz_obj * result = NZ_PULL(self, 0, state_p->chunk_p);
    if(result == NULL) {
        printf("Got NULL\n");
    } else {
        printf("Got chunk\n");
    }

    return NZ_SUCCESS;
}

nz_rc pa_start(struct nz_block * block_p) {
    struct nz_block self = *block_p;
    struct pa_block_state * state_p = (struct pa_block_state *)(self.block_state_p);

    PaError err = Pa_StartStream(state_p->stream);
    if(err != paNoError) NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(Pa_GetErrorText(err)));

    return NZ_SUCCESS;
}

static nz_rc pa_block_create_args(PaDeviceIndex device, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    struct pa_block_state * state_p = calloc(1, sizeof(struct pa_block_state));
    if(state_p == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    state_p->chunk_p = calloc(nz_chunk_size, sizeof(nz_real));

    if(state_p == NULL) {
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
        free(state_p);
        NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(Pa_GetErrorText(err)));
    }

    err = Pa_StartStream(state_p->stream);

    if(err != paNoError) {
        Pa_CloseStream(state_p->stream);
        free(state_p->chunk_p);
        free(state_p);
        NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(Pa_GetErrorText(err)));
    }

    nz_rc rc;
    if((rc = block_info_set_n_io(info_p, 1, 0)) != NZ_SUCCESS ||
       (rc = block_info_set_input(info_p, 0, strdup("in"), &nz_chunk_typeclass, NULL)) != NZ_SUCCESS) {
        block_info_term(info_p);
        Pa_CloseStream(state_p->stream);
        free(state_p->chunk_p);
        free(state_p);
        return rc;
    }

    *(struct pa_block_state **)(state_pp) = state_p;
    return NZ_SUCCESS;
}

nz_rc pa_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    if(string == NULL) NZ_RETURN_ERR(NZ_EXPECTED_BLOCK_ARGS);

    const char * pos = string;
    const char * start;
    size_t length;
    int end;
    nz_rc rc;

    const struct nz_typeclass * typeclass_p;
    nz_type * type_p;

    rc = nz_next_block_arg(string, &pos, &start, &length);
    if(rc != NZ_SUCCESS) return rc;
    char * type_str = strndup(start, length);
    if(type_str == NULL) {
        NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);
    }

    // TODO parse out output device index
    free(type_str);

    if(pos != NULL) {
        NZ_RETURN_ERR_MSG(NZ_OBJ_ARG_PARSE, strdup(string));
    }

    return pa_block_create_args(Pa_GetDefaultOutputDevice(), state_pp, info_p);
}

void pa_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct pa_block_state * pa_block_state_p = (struct pa_block_state *)state_p;

    block_info_term(info_p);
    Pa_CloseStream(&pa_block_state_p->stream);
    free(pa_block_state_p->chunk_p);
    free(pa_block_state_p);
}

DECLARE_BLOCKCLASS(pa)
