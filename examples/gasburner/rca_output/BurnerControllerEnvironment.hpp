#include <cstdlib>
class Random {
    public:
	operator int() const {
		return std::rand();
	}
	operator bool() const {
		return std::rand() % 2;
	}
};

// input functions
