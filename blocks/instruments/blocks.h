#ifndef __BLOCKS_INSTRUMENTS_BLOCKS_H__
#define __BLOCKS_INSTRUMENTS_BLOCKS_H__

#include "core/block.h"

// Basic sine wave instrument
int nz_instrument_sine_init(struct nz_node * node); 

// Basic sawtoohth-approx wave instrument
int nz_instrument_saw_init(struct nz_node * node);

// Enveloped white noise, no pitch
int nz_instrument_snare_init(struct nz_node * node);

#endif
