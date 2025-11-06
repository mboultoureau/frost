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

		DirectX::XMFLOAT3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
		DirectX::XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (auto& v : vertices)
		{
			min.x = min.x < v.position.x ? min.x : v.position.x;
			min.y = min.y < v.position.y ? min.y : v.position.y;
			min.z = min.z < v.position.z ? min.z : v.position.z;

			max.x = max.x > v.position.x ? max.x : v.position.x;
			max.y = max.y > v.position.y ? max.y : v.position.y;
			max.z = max.z > v.position.z ? max.z : v.position.z;
		}

		_boundingBox = { min, max };
	}
}

