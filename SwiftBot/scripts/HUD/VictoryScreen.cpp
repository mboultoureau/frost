#include "VictoryScreen.h"
#include "Frost/Event/Events/PauseMenu/ResetEvent.h"
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"

using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void VictoryScreen::OnCreate()
    {
        auto scene = GetGameObject().GetScene();

        victoryImageId = scene->CreateGameObject("Victory Screen Image");
        restartButtonId = scene->CreateGameObject("Restart Button");

        // Victoire Image (HUDImage dans MainLayer)
        Viewport viewportImage;
        viewportImage.height = 0.7f;
        viewportImage.width = 0.6f;
        viewportImage.x = 0.2f;
        viewportImage.y = 0.1f;

        TextureConfig imageConfig = { .textureType = TextureType::HUD, .path = WIN_PATH };
        auto victoryTexture = Texture::Create(imageConfig);

        auto& imageElement = victoryImageId.AddComponent<UIElement>(
            UIElement{ .content = UIImage{ .texture = victoryTexture, .filter = Material::FilterMode::POINT } });
        imageElement.viewport = viewportImage;
        imageElement.priority = 1;

        // Bouton de Redemarrage (UIButton dans MainLayer)
        Viewport viewportButton;
        viewportButton.height = 0.2f;
        viewportButton.width = 0.2f;
        viewportButton.x = 0.4f;
        viewportButton.y = 0.8f;

        TextureConfig idleConfig = { .textureType = TextureType::HUD, .path = idlePath };
        auto idleTexture = Texture::Create(idleConfig);
        TextureConfig hoverConfig = { .textureType = TextureType::HUD, .path = hoverPath };
        auto hoverTexture = Texture::Create(hoverConfig);
        TextureConfig pressConfig = { .textureType = TextureType::HUD, .path = pressPath };
        auto pressTexture = Texture::Create(pressConfig);

        auto& buttonElement = restartButtonId.AddComponent<UIElement>(
            UIElement{ .content = UIButton{ .idleTexture = idleTexture,
                                            .hoverTexture = hoverTexture,
                                            .pressedTexture = pressTexture,
                                            .idleTextureFilepath = idlePath,
                                            .hoverTextureFilepath = hoverPath,
                                            .pressedTextureFilepath = pressPath,
                                            .onClick = [this]() { OnRestartButtonPress(); } } });
        buttonElement.viewport = viewportButton;
        buttonElement.priority = 1;

        EventManager::Emit<Frost::PauseEvent>();
    }

    void VictoryScreen::OnDestroy()
    {
        auto scene = GetGameObject().GetScene();
        scene->DestroyGameObject(victoryImageId);
        scene->DestroyGameObject(restartButtonId);
    }

    void VictoryScreen::OnRestartButtonPress()
    {
        EventManager::Emit<Frost::ResetEvent>();
    }
} // namespace GameLogic