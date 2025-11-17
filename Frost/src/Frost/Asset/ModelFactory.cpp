#include "Frost/Asset/ModelFactory.h"
#include "Frost/Asset/AssetManager.h"

namespace Frost
{
	std::shared_ptr<Model> ModelFactory::CreateFromFile(const std::string& filepath)
	{
		return AssetManager::LoadAsset<Model>(filepath, filepath);
	}

	std::shared_ptr<Model> ModelFactory::CreateFromHeightMap(HeightMapConfig config)
	{
		return std::make_shared<HeightMapModel>(config);
	}
}