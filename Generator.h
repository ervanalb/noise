#include <vector>

class Generator : public std::vector<void*>
{
	public:
		virtual void get();
};
