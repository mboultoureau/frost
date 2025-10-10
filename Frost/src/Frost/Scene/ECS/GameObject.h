#pragma once

namespace Frost
{
	class GameObject
	{
	public:
		using Id = size_t;

		static const Id InvalidId = static_cast<Id>(0);
	};
}
