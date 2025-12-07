#pragma once

#include "Frost/Renderer/BoundingBox.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Vertex.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace Frost
{
    class Renderer;

    class Mesh
    {
    public:
        Mesh(std::span<const std::byte> vertices, uint32_t vertexStride, std::span<const uint32_t> indices);

        const Buffer* GetVertexBuffer() const { return _vertexBuffer.get(); }
        const Buffer* GetIndexBuffer() const { return _indexBuffer.get(); }

        uint32_t GetVertexStride() const { return _vertexStride; }
        uint32_t GetIndexCount() const { return _indexCount; }
        uint32_t GetMaterialIndex() const { return _materialIndex; }

        void SetMaterialIndex(uint32_t index) { _materialIndex = index; }
        BoundingBox GetBoundingBox() const { return _boundingBox; }

        bool enabled = true;

    private:
        std::shared_ptr<Buffer> _vertexBuffer;
        std::shared_ptr<Buffer> _indexBuffer;
        BoundingBox _boundingBox;

        uint32_t _vertexStride;
        uint32_t _indexCount;
        uint32_t _materialIndex;
    };
} // namespace Frost