#include "SequenceBlock.h"

SequenceBlock::SequenceBlock(std::vector<std::vector<void*> > sequence,std::vector<void*> def) : sequence(sequence),def(def)
{
	out_gen.self=this;
	outputs.push_back(&out_gen); // Only one output port
}

void SequenceBlock::start()
{
	n=0;
}

void SequenceBlock::SequenceGenerator::get()
{
	if(self->n >= (int)self->sequence.size())
	{
		self->out_gen.std::vector<void*>::operator=(self->def);
	}
	else
	{
		self->out_gen.std::vector<void*>::operator=(self->sequence[self->n]);
		self->n++;
	}
}
