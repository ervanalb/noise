#include "OperatorBlock.h"

OperatorBlock::OperatorBlock(void (*operation)(Generator*,Generator*)) : operation(operation)
{
	output_gen.self=this;
	outputs.push_back(&output_gen); // Only one output port
}

void OperatorBlock::start()
{
	input_gen=inputs[0];
}

void OperatorBlock::OpGenerator::get()
{
	self->input_gen->get();
	(*(self->operation))(self->input_gen,&(self->output_gen));
}
