#pragma once

#include "Frost/Core/Application.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Renderer/Model.h"

#include <string>

namespace Frost
{
	class IsHeightMapRenderer{};

	struct ModelRenderer : public Component
	{
		Model* model = nullptr;
		std::string modelFilepath;
		bool isActive = true;
		
		ModelRenderer(std::string filepath)
			: modelFilepath(filepath)
		{
			model = Application::Get().GetModelLibrary().Get(filepath);
		}

		ModelRenderer(IsHeightMapRenderer, std::string filepath, Material& material, TextureChannel channel, int chunksize, float height)
			: modelFilepath(filepath)
		{
			model = Application::Get().GetModelLibrary().MakeHeightMapModel(filepath, material, channel, chunksize, height);
		}
	};
}

