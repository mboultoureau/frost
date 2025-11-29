#pragma once

#include "Frost/Asset/HeightMapModel.h"
#include "Frost/Asset/Model.h"

#include <memory>

namespace Frost
{
    class ModelFactory
    {
    public:
        static std::shared_ptr<Model> CreateFromFile(const std::string& filepath);
        static std::shared_ptr<Model> CreateFromHeightMap(HeightMapConfig config);
        static std::shared_ptr<Model> CreateCube(float size = 2.0f);
        static std::shared_ptr<Model> CreateCubeWithBevel(float size = 2.0f, float bevel = 0.1f);
        static std::shared_ptr<Model> CreateCubeWithPrecision(float size, const Frost::Math::Vector3& nbVertices);
        static std::shared_ptr<Model> CreateSphere(float radius = 1.0f,
                                                   uint32_t sliceCount = 32,
                                                   uint32_t stackCount = 16);
        static std::shared_ptr<Model> CreatePlane(float width = 10.0f, float depth = 10.0f);
        static std::shared_ptr<Model> CreateCylinder(float bottomRadius = 1.0f,
                                                     float topRadius = 1.0f,
                                                     float height = 2.0f,
                                                     uint32_t sliceCount = 32,
                                                     uint32_t stackCount = 1);
    };
} // namespace Frost
