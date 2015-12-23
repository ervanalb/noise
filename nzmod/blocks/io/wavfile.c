#include <stdlib.h>
#include <sndfile.h>

#include "std.h"

struct state {
    SNDFILE * file;
};

int wavfileout_record(struct nz_block * block, nz_real seconds) {
    struct state * state = (struct state *) (block->block_state_p);

    int n_frames = nz_frame_rate * seconds / nz_chunk_size;
    if (n_frames <= 0) return -1;

    int i = 0;
    for (; i < n_frames; i++) {
        nz_real chunk[nz_chunk_size];
        if(NZ_PULL(*block, 0, chunk) == NULL) break;

#ifdef NZ_REAL_FLOAT
        sf_write_float(state->file, chunk, nz_chunk_size);
#else
        sf_write_double(state->file, chunk, nz_chunk_size);
#endif
    }

    sf_write_sync(state->file);
    return i;
}

static nz_rc wavefileout_block_create_args(nz_block_state ** state_pp, struct nz_block_info * info_p, const char * filename) {
    struct state * state = calloc(1, sizeof(struct state));
    if(state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;

    rc = block_info_set_n_io(info_p, 1, 0);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_input(info_p, 0, strdup("in"), &nz_chunk_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    SF_INFO fdata = {
        //.frames = nz_vector_get_size(sample),
        .samplerate = nz_frame_rate,
        .channels = 1,
        .format = SF_FORMAT_WAV | SF_FORMAT_PCM_16,
        //.sections = 1,
        //.seekable = 0,
    };
    state->file = sf_open(filename, SFM_WRITE, &fdata);
    if (state->file == NULL) {
        rc = NZ_INTERNAL_ERROR;
        //int err = sf_error(NULL); // Get error code
        goto fail;
    }

    *(struct state **)(state_pp) = state;
    return NZ_SUCCESS;

fail:
    block_info_term(info_p);
    free(state);
    return rc;

}

static nz_rc wavfileout_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc rc = arg_parse("required string filename", string, args);
    if(rc != NZ_SUCCESS) return rc;

    char * filename = (char *) args[0];
    rc = wavefileout_block_create_args(state_pp, info_p, filename);

    free(filename);
    return rc;
}

static void wavfileout_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct state * state = (struct state *)state_p;

    sf_write_sync(state->file);
    sf_close(state->file);

    block_info_term(info_p);
    free(state);
}

DECLARE_BLOCKCLASS(wavfileout)
