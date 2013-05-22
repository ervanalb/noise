#include "ToneGenBlock.h"
#include <math.h>

ToneGenBlock::ToneGenBlock(float (*waveform)(float)) : waveform(waveform)
{
	output_gen.self=this;
	outputs.push_back(&output_gen); // Only one output port
}

void ToneGenBlock::start()
{
	input_gen=inputs[0];
}

void ToneGenBlock::ToneGenerator::get()
{
	self->input_gen->get();

	size_type len=self->input_gen->size();

	if(len!=self->output_gen.size())
	{
		self->t.resize(len);
		self->osc.resize(len);
		self->output_gen.resize(len);
	}

	for(unsigned i=0;i<len;i++)
	{
		if(self->input_gen->at(i)==0) // null pointer
		{
			self->output_gen[i]=0;
			continue;
		}

		float cur_val=*(float*)(self->input_gen->at(i));

		self->t[i]=fmod((self->t[i]+cur_val),1.0F);
		self->osc[i]=(*(self->waveform))(self->t[i]);
		self->output_gen[i]=&(self->osc[i]);
	}
}
