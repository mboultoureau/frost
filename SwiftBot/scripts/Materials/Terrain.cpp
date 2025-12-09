#include "Materials/Terrain.h"

#include <iostream>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void GameLogic::Terrain::OnCreate()
    {
        TextureConfig rainbowConfig;
        rainbowConfig.textureType = TextureType::DIFFUSE;
        rainbowConfig.path = "./assets/Scenes/RainbowRoad/Terrain/rainbow.jpg";
        std::shared_ptr<Texture> rainbowTexture = Texture::Create(rainbowConfig);

        Material material;
        material.albedoTextures.push_back(rainbowTexture);
        material.albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        material.uvTiling = { 100, 100 };

        auto& staticMesh = GetGameObject().GetComponent<StaticMesh>();
        staticMesh.GetModel()->GetMaterials()[0] = std::move(material);

        FT_INFO("Terrain script: Applied rainbow texture to terrain material.");
    }
} // namespace GameLogic
