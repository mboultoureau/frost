#pragma once

#include "Frost/Asset/AssetManager.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Renderer/Viewport.h"

#include <string>
#include <memory>

namespace Frost::Component
{
    using Vector2 = DirectX::XMFLOAT2;

    class HUDImage : public Component
    {
    public:
        HUDImage() = default;

        HUDImage(
            Viewport viewport,
            std::string pathTexture,
            Material::FilterMode textureFilter
        ) : viewport{ viewport }, textureFilepath{ pathTexture }
        {
			TextureConfig config;
			config.path = pathTexture;
			config.textureType = TextureType::HUD;
            texture = AssetManager::LoadAsset(pathTexture, config);
        }

        const bool IsEnabled() const { return isEnabled; }
		void SetEnabled(bool enabled) { isEnabled = enabled; }

        std::string textureFilepath;
        std::shared_ptr<Texture> texture;
        Material::FilterMode textureFilter;
        Viewport viewport;
        bool isEnabled = true;
    };
}
