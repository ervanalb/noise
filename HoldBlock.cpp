#include "HoldBlock.h"

HoldBlock::HoldBlock(float hold_time) : hold_time(hold_time)
{
	output_gen.self=this;
	outputs.push_back(&output_gen); // Only one output port
}

void HoldBlock::start()
{
	n=0;
	m=0;
	input_gen=inputs[0];
}

void HoldBlock::HoldGenerator::get()
{
	if(self->n>=self->m*self->hold_time)
	{
		self->m++;

		self->input_gen->get();

		if(self->output_gen.size() != self->input_gen->size())
		{
			self->output_gen.resize(self->input_gen->size());
		}
		for(int i=0;i<self->input_gen->size();i++)
		{
			self->output_gen[i]=self->input_gen->at(i);
		}
	}
	self->n++;
}
