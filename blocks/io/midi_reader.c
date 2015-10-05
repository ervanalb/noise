#include <stdlib.h>
#include <string.h>

#include "noise.h"
#include "blocks/io/blocks.h"
#include "blocks/io/midi.h"
#include "blocks/io/midi_smf.h"

struct state {
    FILE * smf_file;
    double last_t;
    uint32_t last_idx;
    struct smf_header * header;
    struct smf_track * track; // Only 1 track for now
};

struct nz_type * nz_midi_vector_type;

static enum nz_pull_rc midireader_pull(struct nz_port * port) {
    struct nz_node * node = port->port_node; 
    struct state * state = (struct state *) node->node_state;

    struct nz_obj * inp_time = NZ_NODE_PULL(node, 0);

    if (inp_time == NULL) {
        return NZ_PULL_RC_NULL;
    }

    double t = NZ_CAST(double, inp_time);
    if (state->last_idx >= state->track->track_nevents) return NZ_PULL_RC_NULL;

    nz_vector_set_size(port->port_value, 0);

    while (state->last_t <= t) {
        double delta_t = t - state->last_t;
        struct smf_event * smf_ev = &state->track->track_events[state->last_idx];
        double event_delta_t = smf_ev->event_deltatime / (double) state->header->header_division;

        if (delta_t >= event_delta_t) {
            state->last_t += event_delta_t;
            state->last_idx++;
            if (state->last_idx >= state->track->track_nevents) return NZ_PULL_RC_NULL;

            // Skip 'meta/sysex' events
            if ((smf_ev->event_data[0] & 0xF0) == 0xF0) continue;

            // Emit event
            struct nz_midiev midi_ev = {
                .midiev_status = smf_ev->event_data[0],
                .midiev_data1 = smf_ev->event_data[1],
                .midiev_data2 = smf_ev->event_length >= 2 ? smf_ev->event_data[2] : 0,
            };

            nz_vector_push_back(port->port_value, &midi_ev);
            //printf("adding ev 0x%02x %u %u %ld\n", midi_ev.midiev_status, midi_ev.midiev_data1, midi_ev.midiev_data2, nz_vector_get_size(port->port_value));
        } else {
            break;
        }
    }

    return NZ_PULL_RC_OBJECT;
}

int nz_midireader_init(struct nz_node * node, const char * filename) {
    if (nz_midi_vector_type == NULL) {
        nz_midi_vector_type = nz_type_create_vector(sizeof(struct nz_midiev));
        if (nz_midi_vector_type == NULL) return -1;
    }

    int rc = nz_node_alloc_ports(node, 1, 1);
    if (rc != 0) return rc;

    node->node_term = &nz_node_term_generic; // FIXME: need to close file handle
    node->node_name = rsprintf("Midi Reader<%s>", filename);

    // Define inputs
    node->node_inputs[0] = (struct nz_inport) {
        .inport_type = nz_double_type,
        .inport_name = strdup("time"),
    };
    
    // Define outputs
    node->node_outputs[0] = (struct nz_port) {
        .port_node = node,
        .port_name = strdup("out"),
        .port_pull = &midireader_pull,
        .port_type = nz_midi_vector_type,
        .port_value = nz_obj_create(nz_midi_vector_type),
    };
    if (node->node_outputs[0].port_value == NULL) 
        return (nz_node_term(node), -1);

    // Initialize state
    node->node_state = calloc(1, sizeof(struct state));
    struct state * state = (struct state *) node->node_state;
    if (state == NULL) 
        return (nz_node_term(node), -1);

    // Open & read header of file
    state->smf_file = fopen(filename, "r");
    state->last_t = 0;

    while (state->header == NULL || state->track == NULL) {
        struct smf_chunk * chunk = smf_read_chunk(state->smf_file);
        if (chunk == NULL) {
            fprintf(stderr, "Not enough chunks in SMF file\n");
            return (nz_node_term(node), -1);
        } else if (memcmp(chunk->chunk_type, "MThd", 4) == 0) {
            if (state->header != NULL) {
                fprintf(stderr, "Multiple headers in SMF file, skipping\n");
                continue;
            }
            state->header = smf_parse_header_chunk(chunk);
            if (state->header == NULL) {
                fprintf(stderr, "Invalid SMF header\n");
                return (nz_node_term(node), -1);
            }
        } else if (memcmp(chunk->chunk_type, "MTrk", 4) == 0) {
            if (state->track != NULL) {
                fprintf(stderr, "Multiple tracks in SMF file, skipping\n");
                continue;
            }
            state->track = smf_parse_track_chunk(chunk);
            if (state->track == NULL) {
                fprintf(stderr, "Invalid SMF track\n");
                return (nz_node_term(node), -1);
            }
        } else {
            fprintf(stderr, "Invalid chunk type.\n");
            return (nz_node_term(node), -1);
        }
    }

    return 0;
}
