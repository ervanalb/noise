/* This was copied from peebles. Not ready yet. */

#include <stdio.h>
#include <math.h>

int rate;
int chunksize;
double attack=0.0000001; // attack exponent base
double sustain=.5; // sustain exponent base
double release=0.0001; // release exponent base
double ttl=1; // seconds to live after release

typedef struct
{
	char active;
	int note;
	double velocity;
	double t;
	double hit;
	double release;
} note_t;

double reduce(double cur,double new)
{
	return cur+new;
}

double f(double t)
{
	//return sin(t*3.14159*2);
	if(fmod(t,1)<0.5)
	{
		return -1;
	}
	return 1;
}

double gen(note_t* note)
{
	double freq=pow(2,(note->note-69)/12.)*440.;
	double base;
	double t_hold;
	double t_release;
	base=f(freq*note->t)*note->velocity;
	t_hold=note->t-note->hit;
	t_release=note->t-note->release;
	if(note->release < note->hit || t_release < 0)
	{
		t_release=0;
	}
	return base*(1-pow(attack,t_hold))*pow(sustain,t_hold)*pow(release,t_release);
}

char still_active(note_t* note)
{
	if(note->release > note->hit && (note->t - note->release) > ttl)
	{
		return 0;
	}
	return 1;
}

void synth(double* buffer,note_t* notes,int num_notes)
{
	int i,j;
	double delta_t=1./rate;

	for(j=0;j<chunksize;j++)
	{
		buffer[j]=0;
	}

	for(i=0;i<num_notes;i++)
	{
		if(!notes[i].active)
		{
			continue;
		}

		for(j=0;j<chunksize;j++)
		{
			buffer[j]=reduce(buffer[j],gen(&(notes[i])));
			notes[i].t+=delta_t;
		}

		notes[i].active=still_active(&(notes[i]));
	}
}
/*
int main()
{
	rate=48000;
	chunksize=2048;

	note_t note[2];

	double buffer[chunksize];

	int i;

	note[0].active=1;
	note[0].note=69;
	note[0].velocity=.1;
	note[0].t=0;
	note[0].hit=0;
	note[0].release=-1;

	note[1].active=1;
	note[1].note=72;
	note[1].velocity=.1;
	note[1].t=0;
	note[1].hit=0;
	note[1].release=-1;

	synth(buffer,note,2);

	for(i=0;i<chunksize;i++)
	{
		printf("%f, ",buffer[i]);
	}
	printf("\n");
}
*/
