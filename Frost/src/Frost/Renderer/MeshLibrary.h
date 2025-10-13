#pragma once

#include "Frost/Renderer/Mesh.h"

#include <string>
#include <unordered_map>

namespace Frost
{
	class MeshLibrary
	{
	public:
		Mesh* Get(const std::string& filepath);
		bool Exists(const std::string& filepath) const;

	private:
		std::unordered_map<std::string, Mesh> _meshes;
	};
}

