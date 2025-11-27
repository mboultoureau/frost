#pragma once

#include "Frost/Renderer/Viewport.h"
#include "Frost/Scene/Components/HUDImage.h"

#include <functional>

namespace Frost::Component
{
    struct UIButton : public HUDImage
    {
        std::shared_ptr<Texture> idleTexture;
        std::shared_ptr<Texture> hoverTexture;
        std::shared_ptr<Texture> pressedTexture;

        std::string hoverTextureFilepath;
        std::string pressedTextureFilepath;

        Viewport buttonHitbox;

        std::function<void()> onClick;

        UIButton(Viewport viewport,
                 std::string pathMainTexture,
                 std::string pathHoverTexture,
                 std::string pathPressedTexture,
                 std::function<void()> f) :
            HUDImage(viewport, pathMainTexture, Material::FilterMode::POINT),
            hoverTextureFilepath(pathHoverTexture),
            pressedTextureFilepath(pathPressedTexture),
            onClick(f)
        {
            idleTexture = texture;

            TextureConfig configHover;
            configHover.path = pathHoverTexture;
            configHover.textureType = TextureType::HUD;
            hoverTexture = AssetManager::LoadAsset(pathHoverTexture, configHover);

            TextureConfig configPressed;
            configPressed.path = pathPressedTexture;
            configPressed.textureType = TextureType::HUD;
            pressedTexture = AssetManager::LoadAsset(pathPressedTexture, configPressed);
        }
    };
} // namespace Frost::Component
