#include "Block.h"

#define FRAMES_PER_BUFFER  (1024)

class PortaudioBlock : public Block
{
	public:
		PortaudioBlock(int);
		void start();
		Generator* input_gen;
        int sample_rate;
	protected:
        /*
		class PortaudioGenerator : public Generator
		{
			public:
				PortaudioBlock* self;
				void get();
		} output_gen;
        */


		void (*operation)(Generator*,Generator*);
        //was static
        //int paCallback( const void *, void *, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void *);

};

