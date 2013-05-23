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
			float* f;
			float f2=0;
			g->get();
			f=(float*)(g->at(j));
			if(f==0)
			{
				outfile.write(&f2,1);
			}
			else
			{
				outfile.write(f,1);
			}
		}
	}
	return 0;
}
