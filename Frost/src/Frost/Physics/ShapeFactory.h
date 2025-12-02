#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include "Frost/Asset/MeshConfig.h"
#include "Frost/Asset/Texture.h"

namespace Frost
{
    class ShapeFactory
    {
    public:
        static JPH::ShapeRefC CreateHeightField(const Component::MeshSourceHeightMap& config,
                                                const std::shared_ptr<Texture>& texture);

    private:
        static float _GetHeightFromPixel(const std::vector<uint8_t>& data,
                                         uint32_t imgW,
                                         uint32_t imgH,
                                         uint32_t channels,
                                         float u,
                                         float v);
    };
} // namespace Frost