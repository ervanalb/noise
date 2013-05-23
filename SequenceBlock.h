#include "Block.h"

class SequenceBlock : public Block
{
	public:
		SequenceBlock(std::vector<std::vector<void*> > sequence,std::vector<void*> def);
		void start();
	protected:
		std::vector<std::vector<void*> > sequence;
		std::vector<void*> def;
		class SequenceGenerator : public Generator
		{
			public:
				SequenceBlock* self;
				void get();
		} out_gen;
		int n;
};
