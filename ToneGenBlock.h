#include "Block.h"

class ToneGenBlock : public Block
{
	public:
		ToneGenBlock(float (*waveform)(float));
		void start();
	protected:
		class ToneGenerator : public Generator
		{
			public:
				ToneGenBlock* self;
				void get();
		} output_gen;

		Generator* input_gen;

		float (*waveform)(float);
		std::vector<float> t,osc;
};
