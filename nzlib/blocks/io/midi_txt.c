#include "blocks/io/midi_smf.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// A shitty text-based format for describing MIDI
/* Example:

16 @ 1/4
0:  #>>> #>>> #>>> #>>>
77: #... #.#. #... #.#.
78: .#>> .#.# .#>> .#.#
*/

#define ERROR(ERRFMT, ...) fprintf(stderr, "Error parsing txtmidi file '%s', line %d: " ERRFMT "\n", filename, lineno, ## __VA_ARGS__)
#define ERRORNO(ERRFMT, ...) fprintf(stderr, "Error parsing txtmidi file '%s', line %d: %s (" ERRFMT ")\n", filename, lineno, strerror(errno), ## __VA_ARGS__)

static int ev_compare(const void * left, const void * right) {
    const struct smf_event * l = left;
    const struct smf_event * r = right;
    if (l->event_deltatime == r->event_deltatime) return 0;
    else if (l->event_deltatime < r->event_deltatime) return -1;
    else return 1;
}

int txtmidi_parse(const char * filename, struct smf_header * header, struct smf_track ** track) {
    char * eventspace = NULL;
    int lineno = 0;
    FILE * file = fopen(filename, "r");
    if (file == NULL) {
        ERRORNO("Unable to open file");
        goto fail;
    }

    char buffer[4096];
    if (fgets(buffer, sizeof buffer, file) == NULL) {
        ERRORNO("Unable to fgets header");
        goto fail;
    }

    unsigned int evcount, evlen, division;
    if (sscanf(buffer, "%u @ %u / %u", &evcount, &evlen, &division) != 3) {
        ERROR("Unable to parse header. Should be in form \"16 @ 1 / 4\". Had \"%s\"", buffer);
        goto fail;
    }

    header->header_ntracks = 1;
    header->header_division = division * 8;

#define MAX_EVENTS 1024
    eventspace = calloc(MAX_EVENTS + 2, sizeof(struct smf_event) + 8);
    *track = (struct smf_track *) eventspace;

    char * evdata = eventspace + (MAX_EVENTS + 2) * sizeof(struct smf_event);
    struct smf_event * events = (*track)->track_events;
    size_t count = 0;
    while (fgets(buffer, sizeof buffer, file) != NULL) {
        int note = 0;
        char * notestr = strtok(buffer, ":"); //XXX
        if (notestr == NULL) {
            ERROR("Parse error");
            goto fail;
        }
        note = atoi(notestr);
        if (note < 0 || note >= 128) {
            ERROR("Invalid note number; expected 0-127, got %d", note);
            goto fail;
        }
        char * evstr = strtok(NULL, "");
        if (evstr == NULL) {
            ERROR("Parse error");
            goto fail;
        }
        unsigned int time = 0;
        bool on = false;

#define ADD_EVENT(status) ({                            \
    *events++ = (struct smf_event) {                    \
        .event_deltatime = time * evlen * 8 - ((status) == 0x80 ? 2 : 0),                \
        .event_length = 3,                              \
        .event_data = evdata                            \
    };                                                  \
    (*track)->track_nevents++;                          \
    *evdata++ = (status);   /* Note on/off  */          \
    *evdata++ = note;       /* note         */          \
    *evdata++ = 127;        /* velocity     */          \
})

        while (*evstr) {
            switch(*evstr) {
            case '#': // Note
                if (on) {
                    ADD_EVENT(0x80);
                    on = false;
                }
                ADD_EVENT(0x90);
                on = true;
                time++;
                break;
            case '>': // Sustain
                if (!on) {
                    ERROR("'>' not preceeded by '#' or '>'");
                    goto fail;
                }
                time++;
                break;
            case '.': // Rest
                if (on) {
                    ADD_EVENT(0x80);
                    on = false;
                }
                time++;
                break;
            default:
                break;
            }
            evstr++;
        }
        if (on) {
            ADD_EVENT(0x80);
            on = false;
        }
        if (time != evcount) {
            ERROR("Invalid number of ticks; expected %d got %d", evcount, time);
            goto fail;
        }
        if ((*track)->track_nevents >= MAX_EVENTS) {
            ERROR("Too many events; MAX_EVENTS is %d", MAX_EVENTS);
            goto fail;
        }
    }

    qsort((*track)->track_events, (*track)->track_nevents, sizeof(struct smf_event), ev_compare);

    // Now convert all the event times into delta times
    for (size_t i = (*track)->track_nevents - 1; i >= 1; i--) {
        (*track)->track_events[i].event_deltatime -=  (*track)->track_events[i-1].event_deltatime;
    }

    return 0;
fail:
    fclose(file);
    free(eventspace);
    return -1;
}
