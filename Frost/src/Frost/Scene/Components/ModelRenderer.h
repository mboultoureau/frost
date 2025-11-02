#pragma once

#include "Frost/Core/Application.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Renderer/Model.h"

#include <string>

namespace Frost
{
	struct ModelRenderer : public Component
	{
		Model* model = nullptr;
		std::string modelFilepath;
		
		ModelRenderer(std::string filepath)
			: modelFilepath(filepath)
		{
			model = Application::Get().GetModelLibrary().Get(filepath);
		}
	};
}

