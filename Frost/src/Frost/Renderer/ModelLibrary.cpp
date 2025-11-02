#include "Frost/Renderer/ModelLibrary.h"

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
}