#pragma once

#include "Frost/Scene/ECS/Component.h"

#include <string>

namespace Frost
{
	struct MeshRenderer : public Component
	{
		std::string meshPath;

		MeshRenderer(std::string path) : meshPath{ path }
		{
		}
	};
}

