#include "UnitImpulseBlock.h"

UnitImpulseBlock::UnitImpulseBlock()
{
	generator.self=this;
	generator.push_back(&out); // Only one result
	outputs.push_back(&generator); // Only one output port
}

void UnitImpulseBlock::start()
{
	t0=true;
}

void UnitImpulseBlock::UnitImpulseGenerator::get()
{
	if(self->t0)
	{
		self->out=1.0;
		self->t0=false;
	}
	else
	{
		self->out=0.0;
	}
}
