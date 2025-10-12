#include "UUID.h"

#include <random>
#include <sstream>

namespace Frost
{
	UUID::UUID()
	{
		static std::random_device rd;
		static std::mt19937_64 prng(rd());
		static std::uniform_int_distribution<uint64_t> distribution;
		_value = distribution(prng);
	}

	UUID::UUID(uint64_t value) : _value(value)
	{
	}

	bool UUID::operator==(const UUID& other) const
	{
		return _value == other._value;
	}
}