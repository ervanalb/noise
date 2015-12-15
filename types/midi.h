#ifndef __TYPES_MIDI_H__
#define __TYPES_MIDI_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "noise.h"

struct nz_midiev {
    unsigned char midiev_status;
    unsigned char midiev_data1;
    unsigned char midiev_data2;
};

extern const struct nz_typeclass nz_midiev_typeclass;

#endif
