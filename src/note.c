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
    

