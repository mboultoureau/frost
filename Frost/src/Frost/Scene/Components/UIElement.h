#pragma once

#include "Frost/Asset/AssetManager.h"
#include "Frost/Asset/Font.h"
#include "Frost/Asset/Texture.h"

#include "Frost/Renderer/Material.h"
#include "Frost/Renderer/Viewport.h"
#include "Frost/Utils/Math/Vector.h"

#include <functional>
#include <memory>
#include <string>
#include <variant>

namespace Frost::Component
{
    struct UIImage
    {
        std::shared_ptr<Texture> texture;
        std::string textureFilepath;
        Material::FilterMode filter = Material::FilterMode::LINEAR;

        void SetTexturePath(const std::string& path)
        {
            textureFilepath = path;
            TextureConfig config = { .textureType = TextureType::HUD, .path = path };
            texture = AssetManager::LoadAsset(path, config);
        }
    };

    struct UIText
    {
        std::string text = "Text";
        std::shared_ptr<Font> font;
        std::string fontFilepath = "./resources/fonts/OpenSans-Regular.ttf";
        float fontSize = 1.0f;
    };

    enum class ButtonState
    {
        Idle,
        Hover,
        Pressed
    };

    struct UIButton
    {
        std::shared_ptr<Texture> idleTexture;
        std::shared_ptr<Texture> hoverTexture;
        std::shared_ptr<Texture> pressedTexture;

        std::string idleTextureFilepath;
        std::string hoverTextureFilepath;
        std::string pressedTextureFilepath;

        std::function<void()> onClick;

        ButtonState state = ButtonState::Idle;
    };

    using UIVariant = std::variant<UIImage, UIText, UIButton>;

    struct UIElement
    {
        Viewport viewport;
        int priority = 0;
        float rotation = 0.0f;
        Math::Color4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        bool isEnabled = true;

        UIVariant content;
    };

} // namespace Frost::Component