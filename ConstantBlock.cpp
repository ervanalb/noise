#include "ConstantBlock.h"

ConstantBlock::ConstantBlock(const std::vector<void*> &c)
{
	generator.std::vector<void*>::operator=(c); // Copy vector into generator
	outputs.push_back(&generator); // Only one output port
}
