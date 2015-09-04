#ifndef __NOTE_H
#define __NOTE_H

typedef struct
{
	enum {NOTE_OFF,NOTE_ON} event;
    int note;
    double velocity;
} note_event_t;

#endif
