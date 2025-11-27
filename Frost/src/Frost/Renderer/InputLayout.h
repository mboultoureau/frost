#pragma once

#include "Frost/Renderer/Format.h"
#include "Frost/Renderer/GPUResource.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/Shader.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Frost
{
    struct VertexAttribute
    {
        std::string name;
        Format format = Format::UNKNOWN;
        uint32_t arraySize = 1;
        uint32_t bufferIndex = 0;
        uint32_t offset = 0;
        uint32_t elementStride = 0;
        bool isInstanced = false;
    };

    class InputLayout : public GPUResource
    {
    public:
        using VertexAttributeArray = std::vector<VertexAttribute>;

        InputLayout(const VertexAttributeArray& attributes, const Shader& shader) {};
        virtual ~InputLayout() = default;
    };
} // namespace Frost