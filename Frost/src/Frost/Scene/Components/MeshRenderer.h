#pragma once

#include "Frost/Core/Application.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Renderer/Mesh.h"

#include <string>

namespace Frost
{
	struct MeshRenderer : public Component
	{
		Mesh* mesh = nullptr;
		std::string meshFilepath;
		
		MeshRenderer(std::string filepath)
			: meshFilepath(filepath)
		{
			mesh = Application::Get().GetMeshLibrary().Get(filepath);
		}
	};
}

