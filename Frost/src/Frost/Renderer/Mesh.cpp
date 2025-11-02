#include "Mesh.h"

namespace Frost
{
	Mesh::Mesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices) : _materialIndex{ 0 }, _indexCount{ 0 }
	{
		UINT vertexBufferSize = static_cast<UINT>(vertices.size_bytes());
		UINT indexBufferSize = static_cast<UINT>(indices.size_bytes());
		size_t indexCount = indices.size();

		_vertexBuffer.Create((void*)vertices.data(), vertexBufferSize);
		_indexBuffer.Create((void*)indices.data(), indexBufferSize, indexCount);

		_indexCount = (uint32_t)indexCount;
	}
}

