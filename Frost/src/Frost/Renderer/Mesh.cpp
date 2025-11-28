#include "Frost/Renderer/Mesh.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
    Mesh::Mesh(std::span<const std::byte> vertices, uint32_t vertexStride, std::span<const uint32_t> indices) :
        _vertexStride(vertexStride), _indexCount(static_cast<uint32_t>(indices.size())), _materialIndex(0)
    {
        FT_ENGINE_ASSERT(!vertices.empty() && !indices.empty(), "Mesh data cannot be empty!");
        FT_ENGINE_ASSERT(vertexStride > 0, "Vertex stride must be greater than zero!");

        BufferConfig vertexBufferConfig = {};
        vertexBufferConfig.usage = BufferUsage::VERTEX_BUFFER;
        vertexBufferConfig.size = static_cast<uint32_t>(vertices.size_bytes());
        vertexBufferConfig.stride = vertexStride;
        vertexBufferConfig.dynamic = false;
        vertexBufferConfig.debugName = "Mesh_VertexBuffer";

        Renderer* renderer = RendererAPI::GetRenderer();
        _vertexBuffer = renderer->CreateBuffer(vertexBufferConfig, vertices.data());

        BufferConfig indexBufferConfig = {};
        indexBufferConfig.usage = BufferUsage::INDEX_BUFFER;
        indexBufferConfig.size = static_cast<uint32_t>(indices.size_bytes());
        indexBufferConfig.stride = sizeof(uint32_t);
        indexBufferConfig.dynamic = false;
        indexBufferConfig.debugName = "Mesh_IndexBuffer";

        _indexBuffer = renderer->CreateBuffer(indexBufferConfig, indices.data());

        DirectX::XMFLOAT3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
        DirectX::XMFLOAT3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        // BoundingBox calculation
        size_t vertexCount = vertices.size_bytes() / vertexStride;
        for (size_t i = 0; i < vertexCount; ++i)
        {
            const std::byte* vertexPtr = vertices.data() + i * vertexStride;
            const float* positionPtr = reinterpret_cast<const float*>(vertexPtr);
            DirectX::XMFLOAT3 position = { positionPtr[0], positionPtr[1], positionPtr[2] };
            min.x = std::min(min.x, position.x);
            min.y = std::min(min.y, position.y);
            min.z = std::min(min.z, position.z);
            max.x = std::max(max.x, position.x);
            max.y = std::max(max.y, position.y);
            max.z = std::max(max.z, position.z);
        }

        _boundingBox = { min, max };
    }
} // namespace Frost