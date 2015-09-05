#ifndef __BLOCKS_IO_BLOCKS_H__
#define __BLOCKS_IO_BLOCKS_H__

#include "core/block.h"

// Soundcard Sink :: (chunk stream) -> ();
struct nz_node * soundcard_get(); // Singleton
void soundcard_run();

#endif
