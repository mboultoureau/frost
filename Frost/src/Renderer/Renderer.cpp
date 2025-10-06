#include "Renderer.h"

namespace Frost
{
	Renderer::Renderer()
	{

	}

	Renderer& Renderer::Get()
	{
		static Renderer instance;
		return instance;
	}
}
