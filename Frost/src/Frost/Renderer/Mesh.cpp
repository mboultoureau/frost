#include "Mesh.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Debugging/Assert.h"

namespace Frost
{
	Mesh::Mesh(std::span<const std::byte> vertices, uint32_t vertexStride, std::span<const uint32_t> indices)
		: _vertexStride(vertexStride),
		_indexCount(static_cast<uint32_t>(indices.size())),
		_materialIndex(0)
	{
		FT_ENGINE_ASSERT(!vertices.empty() && !indices.empty(), "Mesh data cannot be empty!");
		FT_ENGINE_ASSERT(vertexStride > 0, "Vertex stride must be greater than zero!");

		BufferConfig vertexBufferConfig = {};
		vertexBufferConfig.usage = BufferUsage::VERTEX_BUFFER;
		vertexBufferConfig.size = static_cast<uint32_t>(vertices.size_bytes());
		vertexBufferConfig.stride = vertexStride;
		vertexBufferConfig.dynamic = false;

		Renderer* renderer = RendererAPI::GetRenderer();
		_vertexBuffer = renderer->CreateBuffer(vertexBufferConfig, vertices.data());

		BufferConfig indexBufferConfig = {};
		indexBufferConfig.usage = BufferUsage::INDEX_BUFFER;
		indexBufferConfig.size = static_cast<uint32_t>(indices.size_bytes());
		indexBufferConfig.stride = sizeof(uint32_t);
		indexBufferConfig.dynamic = false;

		_indexBuffer = renderer->CreateBuffer(indexBufferConfig, indices.data());


		// _indexCount = (uint32_t)indexCount;

		DirectX::XMFLOAT3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
		DirectX::XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
		
		/*
		for (auto& v : vertices)
		{
			min.x = min.x < v.position.x ? min.x : v.position.x;
			min.y = min.y < v.position.y ? min.y : v.position.y;
			min.z = min.z < v.position.z ? min.z : v.position.z;

			max.x = max.x > v.position.x ? max.x : v.position.x;
			max.y = max.y > v.position.y ? max.y : v.position.y;
			max.z = max.z > v.position.z ? max.z : v.position.z;
		}
		*/

		_boundingBox = { min, max };
	}
}