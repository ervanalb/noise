#ifndef __BLOCKS_IO_MIDI_SMF_H__
#define __BLOCKS_IO_MIDI_SMF_H__

#include <stdint.h>
#include <stdio.h>

struct smf_chunk {
    char chunk_type[4];
    uint32_t chunk_length;
    char chunk_data[0];
};

enum smf_time_format {
    SMF_TIME_METRICAL = 0,
    SMF_TIME_CODEBASED = 1 << 15
};

struct smf_header {
    uint16_t header_format;
    uint16_t header_ntracks;
    uint16_t header_division;
};

struct smf_event {
    uint32_t event_deltatime;
    uint32_t event_length;
    const char * event_data;
};

struct smf_track {
    uint32_t track_nevents;
    struct smf_event track_events[0];
};

uint32_t be32toh(uint32_t x);
uint16_t be16toh(uint16_t x);

int smf_varlen_read(const char * input, uint32_t * value);
int smf_varlen_write(char * output, uint32_t value);

struct smf_chunk * smf_read_chunk(FILE * smf_file);
struct smf_header * smf_parse_header_chunk(const struct smf_chunk * chunk);
struct smf_track * smf_parse_track_chunk(const struct smf_chunk * chunk);

#endif
