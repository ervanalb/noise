#include <sndfile.hh>
#include "Generator.h"

int writeWav(Generator *g,const char* filename,int channels,int sampleRate,int samples)
{
	const int format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	SndfileHandle outfile(filename, SFM_WRITE, format, channels, sampleRate);
	if (not outfile) return -1;

	for(int i=0;i<samples;i++)
	{
		for(int j=0;j<channels;j++)
		{
			g->get();
	    		outfile.write((float*)(g->at(j)),1);
		}
	}
	return 0;
}
