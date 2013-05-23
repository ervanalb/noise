#include "Block.h"

typedef float test_t;

class FIRBlock : public Block
{
	public:
		FIRBlock(std::vector<float> ir);
		void start();
	protected:
		class FIRGenerator : public Generator
		{
			public:
				FIRBlock* self;
				void get();
		} output_gen;

		Generator* input_gen;

		std::vector<float> ir;

		float base;
		std::vector<std::vector<test_t> > memory;
		std::vector<float> result;
};
