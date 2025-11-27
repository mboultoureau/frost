#pragma once

#include "Frost/Renderer/GPUResource.h"
#include "Frost/Renderer/CommandList.h"

#include <cstdint>

namespace Frost
{
    enum class BufferUsage
    {
        UNKNOWN,
        VERTEX_BUFFER,
        INDEX_BUFFER,
        CONSTANT_BUFFER,
    };

    struct BufferConfig
    {
        BufferUsage usage = BufferUsage::UNKNOWN;
        uint32_t size = 0;
        uint32_t stride = 0;
        bool dynamic = false;
        const char* debugName = nullptr;
    };

    class Buffer : public GPUResource
    {
    public:
        virtual ~Buffer() = default;

        virtual const BufferConfig& GetConfig() const = 0;
        virtual void UpdateData(CommandList* commandList, const void* data, uint32_t size, uint32_t offset = 0) = 0;
        virtual uint32_t GetSize() const = 0;
    };
}