#include "Frost/Physics/ShapeFactory.h"
#include "Frost/Debugging/Assert.h"

#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

using namespace Frost::Math;

namespace Frost
{
    JPH::ShapeRefC ShapeFactory::CreateHeightField(const Component::MeshSourceHeightMap& config,
                                                   const std::shared_ptr<Texture>& texture)
    {
        FT_ENGINE_ASSERT(texture, "ShapeFactory: Texture is null.");

        const auto& pixels = texture->GetData();
        uint32_t imgW = texture->GetWidth();
        uint32_t imgH = texture->GetHeight();
        uint32_t channels = texture->GetChannels();
        uint32_t gridResolution = std::max(config.segmentsWidth, config.segmentsDepth) + 1;

        if (gridResolution < 2)
            gridResolution = 2;

        std::vector<float> samples(gridResolution * gridResolution, 0.0f);

        for (uint32_t z = 0; z < gridResolution; ++z)
        {
            for (uint32_t x = 0; x < gridResolution; ++x)
            {
                float u = static_cast<float>(x) / static_cast<float>(gridResolution - 1);
                float v = static_cast<float>(z) / static_cast<float>(gridResolution - 1);
                float hVal = _GetHeightFromPixel(pixels, imgW, imgH, channels, u, v);

                samples[z * gridResolution + x] = hVal;
            }
        }

        float heightRange = config.maxHeight - config.minHeight;

        float scaleX = config.width / float(gridResolution - 1);
        float scaleZ = config.depth / float(gridResolution - 1);

        Vector3 offset(-config.width * 0.5f, config.minHeight, -config.depth * 0.5f);
        Vector3 scale(scaleX, heightRange, scaleZ);

        JPH::HeightFieldShapeSettings settings(
            samples.data(), vector_cast<JPH::Vec3>(offset), vector_cast<JPH::Vec3>(scale), gridResolution);

        JPH::Shape::ShapeResult result = settings.Create();

        if (!result.IsValid())
        {
            FT_ENGINE_ERROR("Failed to create HeightFieldShape: {}", result.GetError().c_str());
            return nullptr;
        }

        return result.Get();
    }

    float ShapeFactory::_GetHeightFromPixel(const std::vector<uint8_t>& data,
                                            uint32_t imgW,
                                            uint32_t imgH,
                                            uint32_t channels,
                                            float u,
                                            float v)
    {
        if (data.empty())
            return 0.0f;

        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        uint32_t x = static_cast<uint32_t>(u * (imgW - 1));
        uint32_t y = static_cast<uint32_t>(v * (imgH - 1));

        size_t index = (y * imgW + x) * channels;
        if (index >= data.size())
            return 0.0f;

        return static_cast<float>(data[index]) / 255.0f;
    }
} // namespace Frost