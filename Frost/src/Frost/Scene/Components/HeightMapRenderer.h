#pragma once

#include "Frost/Core/Application.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Renderer/Model.h"
#include "Frost/Renderer/HeightMapModel.h"

#include <string>

namespace Frost
{
	struct HeightMapRenderer : public Component
	{
		Model* model = nullptr;
		std::string modelFilepath;

		HeightMapRenderer(std::string filepath, Material& material, HeightMapModel::Channel channel, float heightScale)
			: modelFilepath(filepath)
		{
			model = Application::Get().GetModelLibrary().MakeHeightMapModel(filepath, material, channel, heightScale);
		}
	};
}
