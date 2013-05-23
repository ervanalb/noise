#include "FIRBlock.h"
#include <iostream>

using namespace std;

FIRBlock::FIRBlock(std::vector<float> ir) : ir(ir)
{
	output_gen.self=this;
	outputs.push_back(&output_gen); // Only one output port
}

void FIRBlock::start()
{
	input_gen=inputs[0];
}

void FIRBlock::FIRGenerator::get()
{
	self->input_gen->get();

	size_type len=self->input_gen->size();
	size_type ir_len=self->ir.size();

	if(len!=self->output_gen.size())
	{
		self->memory.resize(len);
		self->result.resize(len);
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
			if(ir_len!=self->memory[i].size())
			{
				self->memory[i].resize(ir_len);
			}

			for(unsigned j=0;j<ir_len;j++)
			{
				self->memory[i][j]=cur_val;
			}
		}

		float out=cur_val*self->ir[0];

		for(unsigned j=ir_len-1;j>0;j--)
		{
			self->memory[i][j]=self->memory[i][j-1];
			out+=self->ir[j]*self->memory[i][j];
		}
		self->memory[i][0]=cur_val;
		self->result[i]=out;
		self->output_gen[i]=&(self->result[i]);
	}
}
