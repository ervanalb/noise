#ifndef __BLOCKS_IO_MIDI_H__
#define __BLOCKS_IO_MIDI_H__

#include "noise.h"

struct nz_midiev {
    unsigned char midiev_status;
    unsigned char midiev_data1;
    unsigned char midiev_data2;
};

extern struct nz_type * nz_midi_vector_type;

// MidiReader<filename> :: (double time) -> (vector<midi_event>); Plays an SMF MIDI file
int nz_midireader_init(struct nz_node * node, const char * filename);

// MidiIntegrator :: (vector<midi_event>) -> (vector<notes>); "Integrates" events into notes
int nz_midiintegrator_init(struct nz_node * node);

#endif
