#pragma once

#include <string>

namespace Frost
{
	class Asset
	{
	public:
		using Path = std::string;
		using DebugName = std::string;

		virtual ~Asset() = default;
	};
}
