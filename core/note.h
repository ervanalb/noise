#ifndef __CORE_NOTE_H__
#define __CORE_NOTE_H__

#include "noise.h"
#include "core/ntype.h"

struct nz_note {
    int note_id;
    double note_pitch;
    double note_velocity;
};

void nz_note_init(struct nz_note * note, double pitch, double velocity);
void nz_note_dup(struct nz_note * dst, struct nz_note * src);

struct nz_tnote {
    struct nz_note tnote_note;
    double tnote_start;
    double tnote_duration;
};

void nz_tnote_init(struct nz_tnote * tnote, double pitch, double velocity, double start, double duration);

#endif
