#ifndef __BLOCKS_IO_BLOCKS_H__
#define __BLOCKS_IO_BLOCKS_H__

#include "core/block.h"

// Soundcard Sink :: (chunk stream) -> ();
struct nz_node * soundcard_get(); // Singleton
void soundcard_run();

// Midi Reader (Source)<char * filename> :: (time t) -> (vector<midi_event> evs)
int nz_midi_reader_init(struct nz_node * node, const char * filename);

#endif
