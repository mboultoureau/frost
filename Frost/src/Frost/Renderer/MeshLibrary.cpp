#include "MeshLibrary.h"

namespace Frost
{
	Mesh* MeshLibrary::Get(const std::string& filepath)
	{
		if (!Exists(filepath))
		{
			_meshes.emplace(filepath, Mesh(filepath));
		}
		return &_meshes.at(filepath);
	}

	bool MeshLibrary::Exists(const std::string& filepath) const
	{
		return _meshes.contains(filepath);
	}
}