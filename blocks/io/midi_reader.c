#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "noise.h"
#include "types/ntypes.h"
#include "core/argparse.h"

#include "blocks/io/midi.h"
#include "blocks/io/midi_smf.h"

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) # x

struct state {
    FILE * smf_file;
    nz_real last_t;
    size_t last_idx;
    struct smf_header * header;
    struct smf_track * track; // Only 1 track for now
};

nz_obj * midireader_pull_fn(struct nz_block self, size_t index, nz_obj * obj_p) {
    struct state * state = (struct state *) self.block_state_p;
    nz_real t;

    if(NZ_PULL(self, 0, &t) == NULL) {
        state->last_idx = 0;
        state->last_t = 0;
        return NULL;
    }
    //printf("time t %f\n", t);

    if (state->last_idx >= state->track->track_nevents) return NULL;

    // Has NZ_N_MIDIEVS elements
    struct nz_midiev * output = (struct nz_midiev *) obj_p;
    memset(output, 0, sizeof(*output) * NZ_N_MIDIEVS);

    for (size_t i = 0; i < NZ_N_MIDIEVS && state->last_t <= t; i++) {
        nz_real delta_t = t - state->last_t;
        struct smf_event * smf_ev = &state->track->track_events[state->last_idx];
        nz_real event_delta_t = smf_ev->event_deltatime / (nz_real) state->header->header_division;
        //printf("event_delta_t %f delta_t %f\n", event_delta_t, delta_t);

        if (delta_t >= event_delta_t) {
            state->last_t += event_delta_t;
            state->last_idx++;
            if (state->last_idx >= state->track->track_nevents) return NULL;

            // Skip 'meta/sysex' events
            if ((smf_ev->event_data[0] & 0xF0) == 0xF0) continue;

            // Emit event
            output[i] = (struct nz_midiev) {
                .midiev_status = smf_ev->event_data[0],
                .midiev_data1 = smf_ev->event_data[1],
                .midiev_data2 = smf_ev->event_length >= 2 ? smf_ev->event_data[2] : 0,
            };
            //printf("Put event %#2x %#2x %#2x\n", output[i].midiev_status, output[i].midiev_data1, output[i].midiev_data2);

            //printf("adding ev 0x%02x %u %u %ld\n", midi_ev.midiev_status, midi_ev.midiev_data1, midi_ev.midiev_data2, nz_vector_get_size(port->port_value));
        } else {
            break;
        }
    }

    return obj_p;
}

static nz_rc midireader_block_create_args(nz_block_state ** state_pp, struct nz_block_info * info_p, const struct nz_context * context_p, char * filename) {
    struct state * state = calloc(1, sizeof(struct state));
    if(state == NULL) NZ_RETURN_ERR(NZ_NOT_ENOUGH_MEMORY);

    nz_rc rc;

    const struct nz_typeclass * midiev_array_typeclass;
    nz_type * midiev_array_type;
    rc = nz_type_create(context_p, &midiev_array_typeclass, &midiev_array_type, "array<" STRINGIFY(NZ_N_MIDIEVS) ",midiev>");
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_n_io(info_p, 1, 1);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_input(info_p, 0, strdup("in"), &nz_real_typeclass, NULL);
    if (rc != NZ_SUCCESS) goto fail;

    rc = block_info_set_output(info_p, 0, strdup("out"), 
            midiev_array_typeclass, midiev_array_type,
            midireader_pull_fn);
    if (rc != NZ_SUCCESS) goto fail;

    // Open & read header of file
    state->last_t = 0;
    state->last_idx = 0;

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
    block_info_term(info_p);
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

void midireader_block_destroy(nz_block_state * state_p, struct nz_block_info * info_p) {
    struct state * state = state_p;

    //assert(fclose(state->smf_file) == 0); TODO
    fclose(state->smf_file);
    free(state->header);
    free(state->track);

    block_info_term(info_p);
    free(state_p);
}

DECLARE_BLOCKCLASS(midireader)
