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
		VertexBuffer& GetVertexBuffer() { return _vertexBuffer; }
		IndexBuffer& GetIndexBuffer() { return _indexBuffer; }

	private:
		std::string _filepath;
		VertexBuffer _vertexBuffer;
		IndexBuffer _indexBuffer;
	};
}


