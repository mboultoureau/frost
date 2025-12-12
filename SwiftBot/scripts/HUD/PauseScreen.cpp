#include "PauseScreen.h"
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/ResetEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"
#include "Frost/Input/Devices/Keyboard.h"
#include "Frost/Input/Input.h"

using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void PauseScreen::OnCreate()
    {
        auto scene = GetGameObject().GetScene();

        pauseLogoId = scene->CreateGameObject("Pause Logo");
        resumeButtonId = scene->CreateGameObject("Resume Button");
        resetButtonId = scene->CreateGameObject("Reset Button");
        menuButtonId = scene->CreateGameObject("Menu Button");

        float buttonHeight = 0.25f;
        float buttonWidth = 0.2f;
        float logoHeight = 0.3f;
        float logoWidth = 0.2f;
        float centerbuttonX = 0.5f - (buttonWidth / 2.0f);
        float centerlogoX = 0.5f - (logoWidth / 2.0f);

        Viewport textViewport = { centerlogoX, 0.0f, buttonWidth, logoHeight };
        Viewport resumeViewport = { centerbuttonX, textViewport.y + logoHeight, buttonWidth, buttonHeight };
        Viewport resetViewport = { centerbuttonX, resumeViewport.y + buttonHeight, buttonWidth, buttonHeight };
        Viewport menuViewport = { 0.4, 0.8, 0.2, 0.25 };

        // Pause Logo
        TextureConfig logoConfig = { .textureType = TextureType::HUD, .path = pausePath };
        auto logoTexture = Texture::Create(logoConfig);
        auto& logoElement = pauseLogoId.AddComponent<UIElement>(
            UIElement{ .content = UIImage{ .texture = logoTexture, .filter = Material::FilterMode::POINT } });
        logoElement.viewport = textViewport;

        // Resume Button
        TextureConfig resumeIdleConfig = { .textureType = TextureType::HUD, .path = idleResumePath };
        auto resumeIdleTexture = Texture::Create(resumeIdleConfig);
        TextureConfig resumeHoverConfig = { .textureType = TextureType::HUD, .path = hoverResumePath };
        auto resumeHoverTexture = Texture::Create(resumeHoverConfig);
        TextureConfig resumePressedConfig = { .textureType = TextureType::HUD, .path = pressedResumePath };
        auto resumePressedTexture = Texture::Create(resumePressedConfig);

        auto& resumeElement = resumeButtonId.AddComponent<UIElement>(
            UIElement{ .content = UIButton{ .idleTexture = resumeIdleTexture,
                                            .hoverTexture = resumeHoverTexture,
                                            .pressedTexture = resumePressedTexture,
                                            .idleTextureFilepath = idleResumePath,
                                            .hoverTextureFilepath = hoverResumePath,
                                            .pressedTextureFilepath = pressedResumePath,
                                            .onClick = [this]() { OnUnpauseButtonPress(); } } });
        resumeElement.viewport = resumeViewport;
        resumeElement.priority = 1;

        // Reset Button
        TextureConfig resetIdleConfig = { .textureType = TextureType::HUD, .path = idleResetPath };
        auto resetIdleTexture = Texture::Create(resetIdleConfig);
        TextureConfig resetHoverConfig = { .textureType = TextureType::HUD, .path = hoverResetPath };
        auto resetHoverTexture = Texture::Create(resetHoverConfig);
        TextureConfig resetPressedConfig = { .textureType = TextureType::HUD, .path = pressedResetPath };
        auto resetPressedTexture = Texture::Create(resetPressedConfig);

        auto& resetElement = resetButtonId.AddComponent<UIElement>(
            UIElement{ .content = UIButton{ .idleTexture = resetIdleTexture,
                                            .hoverTexture = resetHoverTexture,
                                            .pressedTexture = resetPressedTexture,
                                            .idleTextureFilepath = idleResetPath,
                                            .hoverTextureFilepath = hoverResetPath,
                                            .pressedTextureFilepath = pressedResetPath,
                                            .onClick = [this]() { OnResetButtonPress(); } } });
        resetElement.viewport = resetViewport;
        resetElement.priority = 1;

        // Exit Button
        TextureConfig exitIdleConfig = { .textureType = TextureType::HUD, .path = idleMenuPath };
        auto exitIdleTexture = Texture::Create(exitIdleConfig);
        TextureConfig exitHoverConfig = { .textureType = TextureType::HUD, .path = hoverMenuPath };
        auto exitHoverTexture = Texture::Create(exitHoverConfig);
        TextureConfig exitPressedConfig = { .textureType = TextureType::HUD, .path = pressedMenuPath };
        auto exitPressedTexture = Texture::Create(exitPressedConfig);

        auto& exitElement = menuButtonId.AddComponent<UIElement>(
            UIElement{ .content = UIButton{ .idleTexture = exitIdleTexture,
                                            .hoverTexture = exitHoverTexture,
                                            .pressedTexture = exitPressedTexture,
                                            .idleTextureFilepath = idleMenuPath,
                                            .hoverTextureFilepath = hoverMenuPath,
                                            .pressedTextureFilepath = pressedMenuPath,
                                            .onClick = [this]() { OnMenuButtonPress(); } } });
        exitElement.viewport = menuViewport;
        exitElement.priority = 1;

        HideMenu();
    }

    void PauseScreen::OnDestroy()
    {
        auto scene = GetGameObject().GetScene();
        scene->DestroyGameObject(pauseLogoId);
        scene->DestroyGameObject(resumeButtonId);
        scene->DestroyGameObject(resetButtonId);
        scene->DestroyGameObject(menuButtonId);
    }

    void PauseScreen::OnUpdate(float deltaTime)
    {
        ProcessInput();
    }

    void PauseScreen::ProcessInput()
    {
        if (Input::GetKeyboard().IsKeyDown(K_ESCAPE) && _pauseButtonReleased)
        {
            _pauseButtonReleased = false;
            _gamePaused = !_gamePaused;

            if (_gamePaused)
            {
                EventManager::Emit<Frost::PauseEvent>();
                ShowMenu();
            }
            else
            {
                EventManager::Emit<Frost::UnPauseEvent>();
                HideMenu();
            }
        }
        else if (!Input::GetKeyboard().IsKeyDown(K_ESCAPE))
        {
            _pauseButtonReleased = true;
        }

        if (Input::GetKeyboard().IsKeyDown(K_R) && _resetButtonReleased)
        {
            EventManager::Emit<Frost::ResetEvent>();
            _resetButtonReleased = false;
        }
        else if (!Input::GetKeyboard().IsKeyDown(K_R))
        {
            _resetButtonReleased = true;
        }
    }

    void PauseScreen::ShowMenu()
    {
        pauseLogoId.SetActive(true);
        resumeButtonId.SetActive(true);
        resetButtonId.SetActive(true);
        menuButtonId.SetActive(true);
    }

    void PauseScreen::HideMenu()
    {
        pauseLogoId.SetActive(false);
        resumeButtonId.SetActive(false);
        resetButtonId.SetActive(false);
        menuButtonId.SetActive(false);
    }

    void PauseScreen::OnUnpauseButtonPress()
    {
        EventManager::Emit<Frost::UnPauseEvent>();
        HideMenu();
        _gamePaused = false;
    }

    void PauseScreen::OnResetButtonPress()
    {
        EventManager::Emit<Frost::ResetEvent>();
        _resetButtonReleased = false;
    }

    void PauseScreen::OnMenuButtonPress()
    {
        EventManager::Emit<Frost::LoadLevelEvent>("assets/Scenes/MainMenu/MainMenu.bin");
    }
} // namespace GameLogic