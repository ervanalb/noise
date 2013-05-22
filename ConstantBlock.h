#include "Block.h"

class ConstantBlock : public Block
{
	public:
		ConstantBlock(const std::vector<void*> &c);
	protected:
		Generator generator;
};
