#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "blocks/io/midi_smf.h"

uint32_t be32toh(uint32_t x) {
    return ((x >> 24) | ((x >> 8) & 0xFF00) | ((x << 8) & 0xFF0000) | (x << 24));
}

uint16_t be16toh(uint16_t x) {
    return ((x >> 8) | (x << 8));
}

int smf_varlen_read(const char * input, uint32_t * value) {
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

int smf_varlen_write(char * output, uint32_t value) {
    uint32_t buffer = value & 0x7F;

    while ((value >>= 7)) {
        buffer <<= 8;
        buffer |= ((value & 0x7F) | 0x80);
    }

    int chars_written = 0;
    while (1) {
        *output++ = buffer & 0xFF;
        chars_written++;

        if (buffer & 0x80) buffer >>= 8;
        else break;
    }

    return chars_written;
}

struct smf_chunk * smf_read_chunk(FILE * smf_file) {
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

struct smf_header * smf_parse_header_chunk(const struct smf_chunk * chunk) {
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

struct smf_track * smf_parse_track_chunk(const struct smf_chunk * chunk) {
    if (memcmp(chunk->chunk_type, "MTrk", 4) != 0) return (errno = EINVAL, NULL);

    // SMF files are kind of terrible at letting you predict the number of events in a track
    // Smallest events are 2 bytes
    struct smf_track * track = calloc(1, sizeof(track) + sizeof(struct smf_event) * (chunk->chunk_length / 2));
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
