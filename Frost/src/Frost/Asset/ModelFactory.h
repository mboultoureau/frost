#pragma once

#include "Frost/Asset/Model.h"
#include "Frost/Asset/HeightMapModel.h"

#include <memory>

namespace Frost
{
	class ModelFactory
	{
	public:
		static std::shared_ptr<Model> CreateFromFile(const std::string& filepath);
		static std::shared_ptr<Model> CreateFromHeightMap(HeightMapConfig config);
	};
}