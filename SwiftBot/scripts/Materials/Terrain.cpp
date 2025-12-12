#include "Materials/Terrain.h"
#include "Physics/PhysicLayer.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <iostream>

using namespace JPH;
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void GameLogic::Terrain::OnCreate()
    {
        TextureConfig textureConfig;
        textureConfig.textureType = TextureType::DIFFUSE;
        textureConfig.path = "./assets/Prefabs/Heightmap/sand_diffuse.jpg";
        std::shared_ptr<Texture> diffuseTexture = Texture::Create(textureConfig);

        textureConfig.textureType = TextureType::NORMALS;
        textureConfig.path = "./assets/Prefabs/Heightmap/sand_normal.jpg";
        std::shared_ptr<Texture> normalTexture = Texture::Create(textureConfig);

        textureConfig.textureType = TextureType::AMBIENT_OCCLUSION;
        textureConfig.path = "./assets/Prefabs/Heightmap/sand_ao.jpg";
        std::shared_ptr<Texture> aoTexture = Texture::Create(textureConfig);

        textureConfig.textureType = TextureType::DIFFUSE_ROUGHNESS;
        textureConfig.path = "./assets/Prefabs/Heightmap/sand_roughness.jpg";
        std::shared_ptr<Texture> roughnessTexture = Texture::Create(textureConfig);

        Material material;
        material.albedoTextures.push_back(diffuseTexture);
        material.normalTextures.push_back(normalTexture);
        material.aoTextures.push_back(aoTexture);
        material.roughnessTextures.push_back(roughnessTexture);
        material.albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        material.uvTiling = { 100, 100 };

        if (GetGameObject().HasComponent<StaticMesh>() == false)
        {
            FT_ENGINE_ERROR("Terrain script: StaticMesh component missing on GameObject.");
            return;
        }

        auto& staticMesh = GetGameObject().GetComponent<StaticMesh>();
        staticMesh.GetModel()->GetMaterials()[0] = std::move(material);

        MeshSourceHeightMap terrainConfig{
            "./assets/Prefabs/Heightmap/heightmap_lake.png", 332.0f, 188.0f, 0.0f, 66.0f, 64, 64
        };

        textureConfig.textureType = TextureType::HEIGHTMAP;
        textureConfig.path = "./assets/Prefabs/Heightmap/heightmap_lake.png";

        std::shared_ptr<Texture> heightmapTexture = Texture::Create(textureConfig);

        ShapeRefC heightFieldShape = ShapeFactory::CreateHeightField(terrainConfig, heightmapTexture);
        auto& transform = GetGameObject().GetParent().GetComponent<Transform>();

        if (heightFieldShape)
        {
            BodyCreationSettings bodySettings(heightFieldShape,
                                              vector_cast<JPH::RVec3>(transform.position),
                                              vector_cast<JPH::Quat>(transform.rotation),
                                              EMotionType::Static,
                                              ObjectLayers::NON_MOVING);
            bodySettings.mRestitution = 0.0f;

            auto bodyId =
                Physics::CreateAndAddBody(bodySettings, GetGameObject().GetHandle(), JPH::EActivation::DontActivate);

            GetGameObject().AddComponent<RigidBody>(bodyId);
        }
        else
        {
            FT_ENGINE_ERROR("Terrain RigidBody creation failed due to null shape.");
        }

        FT_INFO("Terrain script: Applied rainbow texture to terrain material.");
    }
} // namespace GameLogic
