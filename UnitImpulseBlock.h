#include "Block.h"

class UnitImpulseBlock : public Block
{
	public:
		UnitImpulseBlock();
		void start();
	protected:
		bool t0;
		float out;
		class UnitImpulseGenerator : public Generator
		{
			public:
				UnitImpulseBlock* self;
				void get();
		} generator;
};
