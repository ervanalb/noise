#ifndef __CORE_NOTE_H__
#define __CORE_NOTE_H__

#include "noise.h"
#include "core/ntype.h"

struct nz_timing { 
    double timing_begin;
    double timing_end;
};

struct nz_note {
    int note_id;
    double note_pitch;
    double note_velocity;
};

void nz_note_init(struct nz_note * note, double pitch, double velocity);

#endif
