#pragma once

#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Vertex.h"

#include <span>

namespace Frost
{
	class Mesh
	{
	public:
		Mesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices);

		const VertexBuffer& GetVertexBuffer() const { return _vertexBuffer; }
		const IndexBuffer& GetIndexBuffer() const { return _indexBuffer; }
		uint32_t GetMaterialIndex() const { return _materialIndex; }
		void SetMaterialIndex(uint32_t index) { _materialIndex = index; }
		uint32_t GetIndexCount() const { return _indexCount; }

	private:
		VertexBuffer _vertexBuffer;
		IndexBuffer _indexBuffer;
		uint32_t _materialIndex;
		uint32_t _indexCount;
	};
}


