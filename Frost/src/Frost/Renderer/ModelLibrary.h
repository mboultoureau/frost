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

	private:
		std::unordered_map<std::string, Model> _models;
	};
}

