#pragma once

#include "Utils/NoCopy.h"

namespace Frost
{
	class Renderer : NoCopy
	{
	private:
		Renderer();

	public:
		static Renderer& Get();
	};
}


