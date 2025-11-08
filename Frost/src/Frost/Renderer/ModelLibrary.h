#pragma once

#include "Frost/Renderer/Model.h"

#include <string>
#include <unordered_map>

namespace Frost
{
	class ModelLibrary
	{
	public:
		Model* Get(const std::string& filepath);
		bool Exists(const std::string& filepath) const;

		Model* MakeHeightMapModel(const std::string& filepath, const Material& material, const TextureChannel& channel, int chunksize, float height);

	private:
		std::unordered_map<std::string, Model> _models;
	};
}

