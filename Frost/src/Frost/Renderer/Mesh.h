#pragma once

#include "Frost/Renderer/Buffer.h"

#include <string>
#include <unordered_map>

namespace Frost
{
	class Mesh
	{
	public:
		Mesh(const std::string& filepath);
		const std::string& GetFilepath() const { return _filepath; }

	private:
		std::string _filepath;
		VertexBuffer _vertexBuffer;
		IndexBuffer _indexBuffer;
	};

	class MeshLibrary
	{
	public:
		Mesh* Get(const std::string& filepath);
		bool Exists(const std::string& filepath) const;

	private:
		std::unordered_map<std::string, Mesh> _meshes;
	};
}


