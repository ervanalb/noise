#include "Block.h"

class LowPassBlock : public Block
{
	public:
		LowPassBlock(float base);
		void start();
	protected:
		class LowPassGenerator : public Generator
		{
			public:
				LowPassBlock* self;
				void get();
		} output_gen;

		Generator* input_gen;

		float base;
		std::vector<float> filter;
};
