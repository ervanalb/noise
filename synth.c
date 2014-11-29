#include "synth.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "typefns.h"
#include "globals.h"
#include "note.h"

error_t synth_state_alloc(block_info_pt block_info, state_pt* state)
{
    int i;

    synth_info_t* synth_info = block_info;

    synth_state_t* synth_state;

    synth_state = malloc(sizeof(synth_state_t));

    *state = synth_state;

    if(!synth_state) return raise_error(ERR_MALLOC,"");

    synth_state->t = 0;

    synth_state->attack_t = synth_info->attack_t;
    synth_state->attack_amp = synth_info->attack_amp;
    synth_state->decay_t = synth_info->decay_t;
    synth_state->release_t = synth_info->release_t;
    synth_state->num_notes = synth_info->num_notes;

    error_t e;

    e=chunk_alloc(0,(output_pt*)(&(synth_state->chunk)));
    if(e != SUCCESS) return e;

    synth_state->notes=malloc(synth_info->num_notes * sizeof(synth_note_t));
    if(!synth_state->notes) return e;

    for(i=0;i<synth_info->num_notes;i++)
    {
        synth_state->notes[i].active=0;
    }

    return SUCCESS;
}

void synth_state_free(block_info_pt block_info, state_pt state)
{
    chunk_free(0,((synth_state_t*)state)->chunk);
    free(((synth_state_t*)state)->notes);
    free(state);
}

static double sine(double t)
{
    return sin(t*2*M_PI);
}

static double saw(double t)
{
	return 2*fmod(t,1)-1;
}

static double square(double t)
{
	return 2*(fmod(t,1)<.5)-1;
}


static double note2freq(int note)
{
	return pow(2,(double)(note-69)/12)*440;
}

error_t synth_pull(node_t * node, output_pt * output)
{
    note_event_t* note_event;

    error_t e;

    int i,j;

    synth_state_t* state = (synth_state_t*)(node->state);
    synth_note_t* slot;

    for(;;)
    {
        e=pull(node,0,(output_pt*)(&note_event));
        if(e != SUCCESS) return e;
        if(!note_event) break;
        switch(note_event->event)
        {
            case NOTE_ON:
                for(i=0;i<state->num_notes;i++)
                {
                    slot=&state->notes[i];
                    if(!slot->active)
                    {
                        slot->note = note_event->note;
                        slot->velocity = note_event->velocity;
                        slot->start_t = state->t;
                        slot->stop_t = -1;
                        slot->active = 1;
                        break;
                    }
                }
                break;
            case NOTE_OFF:
                for(i=0;i<state->num_notes;i++)
                {
                    slot = &state->notes[i];
                    if(slot->note == note_event->note && slot->active && slot->stop_t < 0)
                    {
                        slot->stop_t = state->t;
                        break;
                    }
                }
                break;
        }
    }

    double envelope;

    for(i=0;i<global_chunk_size;i++)
    {
        state->chunk[i]=0;
        for(j=0;j<state->num_notes;j++)
        {
            slot = &state->notes[j];
            if(slot->active)
            {
                envelope = 1;
                if(state->t - slot->start_t < state->attack_t)
                {
                    envelope = state->attack_amp * (state->t - slot->start_t) / state->attack_t;
                }
                else if(state->t - slot->start_t - state->attack_t < state->decay_t)
                {
                    envelope = (state->attack_amp - (state->attack_amp - 1) * (state->t - slot->start_t - state->attack_t) / state->decay_t);
                }

                if(slot->stop_t > 0 && state->t > slot->stop_t)
                {
                    envelope *= (slot->stop_t + state->release_t - state->t) / state->release_t;
                    if(envelope < 0)
                    {
                        slot->active = 0;
                        continue;
                    }
                }
                state->chunk[i] += saw((state->t - slot->start_t) * note2freq(slot->note)) * envelope * slot->velocity;
            }
        }
        state->t += 1 / global_frame_rate;
    }

    *output = ((void *) (state->chunk));

    return SUCCESS;
}

