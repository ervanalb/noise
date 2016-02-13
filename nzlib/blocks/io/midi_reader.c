#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "std.h"
#include "blocks/io/midi_smf.h"

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) # x

struct state {
    FILE * smf_file;
    nz_real last_t;
    enum midi_continuation {
        CONT_NONE,
        CONT_TIMED,
        CONT_REMAINING
    } continuation;
    size_t last_idx;
    struct smf_header * header;
    struct smf_track * track; // Only 1 track for now
};

nz_obj * midireader_next(struct state * state, enum midi_continuation cont, nz_obj * obj_p) {
    struct nz_midiev * output = (struct nz_midiev *) obj_p;
    struct smf_event * smf_ev;
    do {
        if (state->last_idx >= state->track->track_nevents) return NULL;
        smf_ev = &state->track->track_events[state->last_idx];
        state->last_idx++;

        // Skip 'meta/sysex' events
    } while ((smf_ev->event_data[0] & 0xF0) == 0xF0);

    // Emit event
    *output = (struct nz_midiev) {
        .midiev_status = smf_ev->event_data[0],
        .midiev_data1 = smf_ev->event_data[1],
        .midiev_data2 = smf_ev->event_length >= 2 ? smf_ev->event_data[2] : 0,
    };
    state->continuation = cont;

    /*
    char * midiev_str = NULL;
    if (nz_midiev_typeclass.type_str_obj(NULL, output, &midiev_str) == NZ_SUCCESS) {
        printf("Put event %s %lf\n", midiev_str, state->last_t);
        free(midiev_str);
    } else {
        printf("Put event (unprintable)\n");
    }
    */

    return obj_p;
}

nz_obj * midireader_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state = (struct state *) self.block_state_p;
    nz_real t;

    if (state->continuation == CONT_REMAINING) {
        if (state->last_idx < state->track->track_nevents) {
            return midireader_next(state, CONT_REMAINING, obj_p);
        }
    } else if (state->continuation == CONT_NONE) {
        if(NZ_PULL(self, 0, &t) == NULL) {
            state->last_idx = 0;
            state->last_t = 0;
            return NULL;
        }

        if (t < state->last_t) {
            if (state->last_idx > 0 && state->last_idx < state->track->track_nevents) {
                //printf("triggering cont remaining %lf %lf\n", t, state->last_t);
                return midireader_next(state, CONT_REMAINING, obj_p);
            }
            state->last_idx = 0;
            state->last_t = 0;
        }
    }

    state->continuation = CONT_NONE;

    while (state->last_t <= t) {
        nz_real delta_t = t - state->last_t;
        struct smf_event * smf_ev = &state->track->track_events[state->last_idx];
        nz_real event_delta_t = smf_ev->event_deltatime / (nz_real) state->header->header_division;

        if (delta_t < event_delta_t) return NULL;
        state->last_t += event_delta_t;

        if ((smf_ev->event_data[0] & 0xF0) == 0xF0) {
            // Skip sysex events
            state->last_idx++;
        } else {
            return midireader_next(state, CONT_TIMED, obj_p);
        }
    }

    return NULL;
}

static nz_rc midireader_block_create_args(nz_block_state ** state_pp, struct nz_block_info * info_p, const struct nz_context * context_p, char * filename) {
    struct state * state = calloc(1, sizeof(struct state));
    if(state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;

    rc = nz_block_info_set_n_io(info_p, 1, 1);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_input(info_p, 0, strdup("in"), &nz_real_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    rc = nz_block_info_set_output(info_p, 0, strdup("out"), &nz_midiev_typeclass, NULL, midireader_pull_fn); 
    if (rc != NZ_SUCCESS) goto fail;

    // Open & read header of file
    state->last_t = 0;
    state->last_idx = 0;
    state->continuation = CONT_NONE;

    state->smf_file = fopen(filename, "r");
    if (state->smf_file == NULL) {
        NZ_RETURN_ERR_MSG(NZ_INTERNAL_ERROR, strdup(strerror(errno)));
    }

    while (state->header == NULL || state->track == NULL) {
        struct smf_chunk * chunk = smf_read_chunk(state->smf_file);
        if (chunk == NULL) {
            fprintf(stderr, "Not enough chunks in SMF file\n");
            rc = NZ_INTERNAL_ERROR;
            goto fail;
        } else if (memcmp(chunk->chunk_type, "MThd", 4) == 0) {
            if (state->header != NULL) {
                fprintf(stderr, "Multiple headers in SMF file, skipping\n");
                continue;
            }
            state->header = smf_parse_header_chunk(chunk);
            if (state->header == NULL) {
                fprintf(stderr, "Invalid SMF header\n");
                rc = NZ_INTERNAL_ERROR;
                goto fail;
            }
        } else if (memcmp(chunk->chunk_type, "MTrk", 4) == 0) {
            if (state->track != NULL) {
                fprintf(stderr, "Multiple tracks in SMF file, skipping\n");
                continue;
            }
            state->track = smf_parse_track_chunk(chunk);
            if (state->track == NULL) {
                fprintf(stderr, "Invalid SMF track\n");
                rc = NZ_INTERNAL_ERROR;
                goto fail;
            }
        } else {
            fprintf(stderr, "Invalid chunk type.\n");
            rc = NZ_INTERNAL_ERROR;
            goto fail;
        }
    }

    *(struct state **)(state_pp) = state;
    return NZ_SUCCESS;

fail:
    free(state);
    return rc;
}

nz_rc midireader_block_create(const struct nz_context * context_p, const char * string, nz_block_state ** state_pp, struct nz_block_info * info_p) {
    nz_arg * args[1];
    nz_rc result = arg_parse("required string filename", string, args);
    if(result != NZ_SUCCESS) return result;

    char * filename_str = (char *)args[0];
    nz_rc rc = midireader_block_create_args(state_pp, info_p, context_p, filename_str);
    free(filename_str);

    return rc;
}

void midireader_block_destroy(nz_block_state * state_p) {
    struct state * state = state_p;

    //assert(fclose(state->smf_file) == 0); TODO
    fclose(state->smf_file);
    free(state->header);
    free(state->track);

    free(state_p);
}

NZ_DECLARE_BLOCKCLASS(midireader)
