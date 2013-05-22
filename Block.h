#ifndef _GENERATOR_H
#define _GENERATOR_H

#include "Generator.h"
#include <vector>

class Block
{
	public:
		std::vector<Generator*> inputs;
		std::vector<Generator*> outputs;

		virtual void start();
};

#endif
