#include "Materials/Terrain.h"

#include <iostream>

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

        auto& staticMesh = GetGameObject().GetComponent<StaticMesh>();
        staticMesh.GetModel()->GetMaterials()[0] = std::move(material);

        FT_INFO("Terrain script: Applied rainbow texture to terrain material.");
    }
} // namespace GameLogic
