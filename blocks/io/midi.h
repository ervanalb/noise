#ifndef __BLOCKS_IO_MIDI_H__
#define __BLOCKS_IO_MIDI_H__

#include "noise.h"

#define NZ_N_MIDIEVS 128

// MidiReader<filename> :: (double time) -> (array<128,midiev>); Plays an SMF MIDI file
const struct nz_blockclass nz_midireader_blockclass;

// MidiMelody<> :: (array<128,midiev>) -> (real pitch, real velocity); Extract melody from MIDI stream
const struct nz_blockclass nz_midimelody_blockclass;

//// MidiIntegrator :: (vector<midi_event>) -> (vector<notes>); "Integrates" events into notes
//int nz_midiintegrator_init(struct nz_node * node);

#endif
