#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "noise.h"
#include "core/ntype.h"
#include "core/util.h"

static int next_note_id = 0;

void nz_note_init(struct nz_note * note, double pitch, double velocity) {
    note->note_id = next_note_id++;
    note->note_pitch = pitch;
    note->note_velocity = velocity;
}

void nz_note_dup(struct nz_note * dst, struct nz_note * src) {
    nz_note_init(dst, src->note_pitch, src->note_velocity);
}

void nz_tnote_init(struct nz_tnote * tnote, double pitch, double velocity, double start, double duration) {
    nz_note_init(&tnote->tnote_note, pitch, velocity);
    tnote->tnote_start = start;
    tnote->tnote_duration = duration;
}
