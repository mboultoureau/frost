#include "Frost/Renderer/ModelLibrary.h"
#include "Frost/Renderer/HeightMapModel.h"

namespace Frost
{
	Model* ModelLibrary::Get(const std::string& filepath)
	{
		if (!Exists(filepath))
		{
			_models.emplace(filepath, Model(filepath));
		}
		return &_models.at(filepath);
	}

	bool ModelLibrary::Exists(const std::string& filepath) const
	{
		return _models.contains(filepath);
	}

	Model* ModelLibrary::MakeHeightMapModel(const std::string& filepath, const Material& material, const TextureChannel& channel, int chunksize, float height)
	{
		if (!Exists(filepath))
		{
			_models.emplace(filepath, HeightMapModel(filepath, material, channel, chunksize, height));
		}
		return &_models.at(filepath);
	}
}