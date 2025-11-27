#pragma once

#include "Frost/Asset/Model.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Utils/Math/Vector.h"

#include <string>

namespace Frost
{
    struct HeightMapConfig
    {
        Material material;
        std::shared_ptr<Texture> texture;
        uint32_t chunkSize = 10;
        Math::Vector3 scale = { 1.0f, 1.0f, 1.0f };
    };

    class HeightMapModel : public Model
    {
    public:
        HeightMapModel(const HeightMapConfig& config);

    private:
        void GenerateHeightMapMesh(uint32_t xMin, uint32_t xMax, uint32_t zMin, uint32_t zMax, float heightScale);

    private:
        HeightMapConfig _config;
    };
} // namespace Frost