#pragma once

#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Vertex.h"
#include "Frost/Renderer/BoundingBox.h"

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
		BoundingBox GetBoundingBox() const { return _boundingBox; }

	private:
		VertexBuffer _vertexBuffer;
		IndexBuffer _indexBuffer;
		BoundingBox _boundingBox;
		uint32_t _materialIndex;
		uint32_t _indexCount;
	};
}


