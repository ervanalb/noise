#include "Block.h"

class HoldBlock : public Block
{
	public:
		HoldBlock(float hold_time);
		void start();
	protected:
		class HoldGenerator : public Generator
		{
			public:
				HoldBlock* self;
				void get();
		} output_gen;

		int n,m;
		float hold_time;

		Generator* input_gen;
};
