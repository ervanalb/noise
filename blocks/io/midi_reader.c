#include <stdlib.h>
#include <string.h>

#include "noise.h"
#include "blocks/io/blocks.h"
#include "blocks/io/midi.h"
#include "blocks/io/midi_smf.h"

static uint32_t be32toh(uint32_t x) {
    return ((x >> 24) | ((x >> 8) & 0xFF00) | ((x << 8) & 0xFF0000) | (x << 24));
}

static uint16_t be16toh(uint16_t x) {
    return ((x >> 8) | (x << 8));
}

struct state {
    FILE * smf_file;
    double last_t;
    uint32_t last_idx;
    struct smf_header * header;
    struct smf_track * track; // Only 1 track for now
};

struct nz_type * nz_midi_vector_type;

static int smf_varlen_read(const char * input, uint32_t * value) {
    unsigned char c;
    int chars_read = 1;
    assert(value != NULL);

    if ((*value = *input++) & 0x80) {
        *value &= 0x7F;
        do {
            *value = (*value << 7) + ((c = *input++) & 0x7F);
            chars_read++;
        } while (c & 0x80); 
    }

    return chars_read;
}

static struct smf_chunk * smf_read_chunk(FILE * smf_file) {
    char chunk_type[4];
    size_t rc = fread(chunk_type, 1, 4, smf_file);
    if (rc != 4) return NULL;

    uint32_t chunk_length;
    rc = fread(&chunk_length, 1, 4, smf_file);
    if (rc != 4) return NULL;

    chunk_length = be32toh(chunk_length);

    struct smf_chunk * chunk = malloc(chunk_length + sizeof(chunk));
    if (chunk == NULL) return NULL;

    chunk->chunk_length = chunk_length;
    memcpy(chunk->chunk_type, chunk_type, 4);

    rc = fread(chunk->chunk_data, 1, chunk_length, smf_file);
    if (rc != chunk_length) return (free(chunk), NULL);

    return chunk;
}

static struct smf_header * smf_parse_header_chunk(const struct smf_chunk * chunk) {
    if (memcmp(chunk->chunk_type, "MThd", 4) != 0) return (errno = EINVAL, NULL);

    struct smf_header * header = malloc(sizeof(header));
    if (header == NULL) return NULL;

    uint16_t value;
    memcpy(&value, &chunk->chunk_data[0 * sizeof(value)], sizeof(value));
    value = be16toh(value);
    header->header_format = value;

    memcpy(&value, &chunk->chunk_data[1 * sizeof(value)], sizeof(value));
    value = be16toh(value);
    header->header_ntracks = value;

    memcpy(&value, &chunk->chunk_data[2 * sizeof(value)], sizeof(value));
    value = be16toh(value);
    header->header_division = value;

    return header;
}

static struct smf_track * smf_parse_track_chunk(const struct smf_chunk * chunk) {
    if (memcmp(chunk->chunk_type, "MTrk", 4) != 0) return (errno = EINVAL, NULL);

    // SMF files are kind of terrible at letting you predict the number of events in a track
    // Smallest events are 2 bytes
    struct smf_track * track = malloc(sizeof(track) + sizeof(struct smf_event) * (chunk->chunk_length / 2));
    if (track == NULL) return NULL;

    size_t offset = 0;
    while (offset < chunk->chunk_length) {
        struct smf_event * event = &track->track_events[track->track_nevents++];

        int rc = smf_varlen_read(chunk->chunk_data + offset, &event->event_deltatime);
        if (rc < 0) return (free(track), NULL);

        event->event_data = (const char *) (chunk->chunk_data + (offset += rc));
        unsigned char status = event->event_data[0];
        if (!(status & 0x80)) { // If the high bit isn't set
            fprintf(stderr, "Invalid status 0x%02X\n", status);
            return (free(track), NULL);
        } else if ((status & 0xF0) == 0xF0) {
            if (status == 0xF0) { // Sysex : <status> <length : variable> <data>
                uint32_t event_length;
                int rc = smf_varlen_read(event->event_data + 1, &event_length);
                if (rc < 0) return (free(track), NULL);
                event->event_length = event_length + 1 + rc;
            } else if (status == 0xFF) { // Meta
                uint32_t event_length;
                unsigned char meta = event->event_data[1];
                int rc = smf_varlen_read(event->event_data + 2, &event_length);
                if (rc < 0) return (free(track), NULL);
                event->event_length = event_length + 2 + rc;
            } else {
                fprintf(stderr, "Unhandled event 0x%02X\n", status);
                return (free(track), NULL);
            }
        } else {
            switch (status & 0xF0) {
                case 0x80:
                case 0x90:
                case 0xA0:
                case 0xB0:
                case 0xE0:
                    event->event_length = 3;
                    break;
                case 0xC0:
                case 0xD0:
                    event->event_length = 2;
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        offset += event->event_length;
    }
    return track;
}

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
            printf("adding ev 0x%02x %u %u %ld\n", midi_ev.midiev_status, midi_ev.midiev_data1, midi_ev.midiev_data2, nz_vector_get_size(port->port_value));
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
