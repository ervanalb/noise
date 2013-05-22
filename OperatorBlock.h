#include "Block.h"

class OperatorBlock : public Block
{
	public:
		OperatorBlock(void (*operation)(Generator*,Generator*));
		void start();
	protected:
		class OpGenerator : public Generator
		{
			public:
				OperatorBlock* self;
				void get();
		} output_gen;

		Generator* input_gen;

		void (*operation)(Generator*,Generator*);
};
