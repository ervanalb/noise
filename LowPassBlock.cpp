#include "LowPassBlock.h"

LowPassBlock::LowPassBlock(float base) : base(base)
{
	output_gen.self=this;
	outputs.push_back(&output_gen); // Only one output port
}

void LowPassBlock::start()
{
	input_gen=inputs[0];
}

void LowPassBlock::LowPassGenerator::get()
{
	self->input_gen->get();

	size_type len=self->input_gen->size();

	if(len!=self->output_gen.size())
	{
		self->filter.resize(len);
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

		if(self->output_gen[i]==0) // null pointer
		{
			self->filter[i]=cur_val;
		}

		self->filter[i]=cur_val*(1-self->base)+self->filter[i]*self->base;
		self->output_gen[i]=&(self->filter[i]);
	}
}
