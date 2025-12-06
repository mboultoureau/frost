#pragma once

#include "Frost/Asset/AssetManager.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Renderer/Viewport.h"
#include "Frost/Scene/ECS/Component.h"

#include <memory>
#include <string>

namespace Frost::Component
{
    using Vector2 = DirectX::XMFLOAT2;

    class HUDImage : public Component
    {
    public:
        HUDImage() = default;

        HUDImage(Viewport viewport, std::string pathTexture, Material::FilterMode textureFilter) :
            viewport{ viewport }, textureFilepath{ pathTexture }
        {
            TextureConfig config;
            config.path = pathTexture;
            config.textureType = TextureType::HUD;
            texture = AssetManager::LoadAsset(pathTexture, config);
        }

        void SetTexturePath(const std::string& newPath)
        {
            textureFilepath = newPath;

            TextureConfig config;
            config.path = newPath;
            config.textureType = TextureType::HUD;

            texture = AssetManager::LoadAsset(textureFilepath, config);
        }

        std::string textureFilepath;
        std::shared_ptr<Texture> texture;
        Material::FilterMode textureFilter;
        Viewport viewport;
        float rotation = 0.0f;
    };
} // namespace Frost::Component
