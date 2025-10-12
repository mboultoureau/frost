#pragma once

#include <cstdint>

namespace Frost
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t value);
		bool operator==(const UUID& other) const;

	private:
		uint64_t _value;
	};
}
