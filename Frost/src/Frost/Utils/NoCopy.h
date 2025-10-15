#pragma once

namespace Frost
{
	class NoCopy
	{
	public:
		NoCopy(const NoCopy&) = delete;
		NoCopy& operator=(const NoCopy&) = delete;

	protected:
		constexpr NoCopy() = default;
		~NoCopy() = default;
	};
}
