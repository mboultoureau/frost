#include "HUD/SplashScreen.h"
#include "GameState/GameState.h"
using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void SplashScreen::OnCreate()
    {
        auto scene = GetGameObject().GetScene();

        titleImageId = scene->CreateGameObject("Start Title");
        creditsImageId = scene->CreateGameObject("Credits Title");
        startButtonId = scene->CreateGameObject("Start Button");
        creditsButtonId = scene->CreateGameObject("Credit Button");
        backButtonId = scene->CreateGameObject("Back Button");
        creditsText1 = scene->CreateGameObject("Text");
        creditsText2 = scene->CreateGameObject("Text");
        exitButtonId = scene->CreateGameObject("Exit Button");

        Viewport imageViewport;
        imageViewport.height = 1.0f;
        imageViewport.width = 1.0f;
        imageViewport.x = 0.0f;
        imageViewport.y = 0.0f;

        // Title
        TextureConfig config = { .textureType = TextureType::HUD, .path = titlePath };
        auto titleTexture = Texture::Create(config);

        auto& titleElement = titleImageId.AddComponent<UIElement>(
            UIElement{ .content = UIImage{ .texture = titleTexture, .filter = Material::FilterMode::POINT } });
        titleElement.viewport = imageViewport;

        // Credits
        config = { .textureType = TextureType::HUD, .path = creditsPath };
        auto creditsTexture = Texture::Create(config);

        auto& creditElement = creditsImageId.AddComponent<UIElement>(
            UIElement{ .content = UIImage{ .texture = creditsTexture, .filter = Material::FilterMode::POINT } });
        creditElement.viewport = imageViewport;

        // Start button
        config = { .textureType = TextureType::HUD, .path = idleStartPath };
        auto startIdleTexture = Texture::Create(config);

        config = { .textureType = TextureType::HUD, .path = hoverStartPath };
        auto startHoverTexture = Texture::Create(config);

        config = { .textureType = TextureType::HUD, .path = pressedStartPath };
        auto startPressedTexture = Texture::Create(config);

        auto& startButtonElement = startButtonId.AddComponent<UIElement>(
            UIElement{ .content = UIButton{ .idleTexture = startIdleTexture,
                                            .hoverTexture = startHoverTexture,
                                            .pressedTexture = startPressedTexture,
                                            .idleTextureFilepath = idleStartPath,
                                            .hoverTextureFilepath = hoverStartPath,
                                            .pressedTextureFilepath = pressedStartPath,
                                            .onClick = [this]() { OnStartButtonPress(); } } });
        startButtonElement.priority = 1;

        Viewport buttonPlayViewport;
        buttonPlayViewport.height = 0.25f;
        buttonPlayViewport.width = 0.2f;
        buttonPlayViewport.x = 0.42f;
        buttonPlayViewport.y = 0.74f;

        startButtonElement.viewport = buttonPlayViewport;

        // Exit button
        config = { .textureType = TextureType::HUD, .path = idleExitPath };
        auto exitIdleTexture = Texture::Create(config);

        config = { .textureType = TextureType::HUD, .path = hoverExitPath };
        auto exitHoverTexture = Texture::Create(config);

        config = { .textureType = TextureType::HUD, .path = pressedExitPath };
        auto exitPressedTexture = Texture::Create(config);

        auto& exitButtonElement = exitButtonId.AddComponent<UIElement>(
            UIElement{ .content = UIButton{ .idleTexture = exitIdleTexture,
                                            .hoverTexture = exitHoverTexture,
                                            .pressedTexture = exitPressedTexture,
                                            .idleTextureFilepath = idleExitPath,
                                            .hoverTextureFilepath = hoverExitPath,
                                            .pressedTextureFilepath = pressedExitPath,
                                            .onClick = [this]() { OnExitButtonPress(); } } });
        exitButtonElement.priority = 1;

        Viewport buttonExitViewport;
        buttonExitViewport.height = 0.25f;
        buttonExitViewport.width = 0.2f;
        buttonExitViewport.x = 0.02f;
        buttonExitViewport.y = 0.82f;

        exitButtonElement.viewport = buttonExitViewport;

        // Credits
        config = { .textureType = TextureType::HUD, .path = idleCreditsPath };
        auto creditsIdleTexture = Texture::Create(config);

        config = { .textureType = TextureType::HUD, .path = hoverCreditsPath };
        auto creditsHoverTexture = Texture::Create(config);

        config = { .textureType = TextureType::HUD, .path = pressedCreditsPath };
        auto creditsPressedTexture = Texture::Create(config);

        auto& creditsElement = creditsButtonId.AddComponent<UIElement>(
            UIElement{ .content = UIButton{ .idleTexture = creditsIdleTexture,
                                            .hoverTexture = creditsHoverTexture,
                                            .pressedTexture = creditsPressedTexture,
                                            .idleTextureFilepath = idleCreditsPath,
                                            .hoverTextureFilepath = hoverCreditsPath,
                                            .pressedTextureFilepath = pressedCreditsPath,
                                            .onClick = [this]() { OnCreditsButtonPress(); } } });
        creditsElement.priority = 1;

        Viewport buttonCreditsViewport;
        buttonCreditsViewport.height = 0.25f;
        buttonCreditsViewport.width = 0.18f;
        buttonCreditsViewport.x = 0.82f;
        buttonCreditsViewport.y = 0.82f;
        creditsElement.viewport = buttonCreditsViewport;

        // Back
        config = { .textureType = TextureType::HUD, .path = idleBackPath };
        auto backIdleTexture = Texture::Create(config);

        config = { .textureType = TextureType::HUD, .path = hoverBackPath };
        auto backHoverTexture = Texture::Create(config);

        config = { .textureType = TextureType::HUD, .path = pressedBackPath };
        auto backPressedTexture = Texture::Create(config);

        auto& backButtonElement = backButtonId.AddComponent<UIElement>(
            UIElement{ .content = UIButton{ .idleTexture = backIdleTexture,
                                            .hoverTexture = backHoverTexture,
                                            .pressedTexture = backPressedTexture,
                                            .idleTextureFilepath = idleBackPath,
                                            .hoverTextureFilepath = hoverBackPath,
                                            .pressedTextureFilepath = pressedBackPath,
                                            .onClick = [this]() { OnBackButtonPress(); } } });
        Viewport buttonBackViewport;
        buttonBackViewport.height = 0.25f;
        buttonBackViewport.width = 0.18f;
        buttonBackViewport.x = 0.07f;
        buttonBackViewport.y = 0.08f;

        backButtonElement.priority = 1;
        backButtonElement.viewport = buttonBackViewport;

        // Credits Text
        auto font = AssetManager::LoadAsset<Font>("assets/HUD/ss-engine.regular.ttf");
        std::string initText1 = "ELIAS DEL POZO                            SIMON LE FLOCH";

        auto& creditsText1Element = creditsText1.AddComponent<UIElement>(
            UIElement{ .content = UIText{ .text = initText1,
                                          .font = font,
                                          .fontFilepath = "assets/HUD/ss-engine.regular.ttf",
                                          .fontSize = 1.1f } });

        Viewport viewportText1;
        viewportText1.x = 0.1f;
        viewportText1.y = 0.85f;
        viewportText1.width = 0.5f;
        viewportText1.height = 0.5f;
        creditsText1Element.viewport = viewportText1;
        creditsText1Element.priority = 1;

        // Credits Text 2
        std::string initText2 = "MATHIS BOULTOUREAU                   THOMAS VANWALLEGHEM";

        auto& creditsText2Element = creditsText2.AddComponent<UIElement>(
            UIElement{ .content = UIText{ .text = initText2,
                                          .font = font,
                                          .fontFilepath = "resources/fonts/ss-engine.regular.ttf",
                                          .fontSize = 1.1f } });

        Viewport viewportText2;
        viewportText2.x = 0.1f;
        viewportText2.y = 0.9f;
        viewportText2.width = 0.5f;
        viewportText2.height = 0.5f;
        creditsText2Element.viewport = viewportText2;
        creditsText2Element.priority = 1;

        ShowMenu();
    }

    void SplashScreen::OnDestroy()
    {
        auto scene = GetGameObject().GetScene();
        scene->DestroyGameObject(titleImageId);
        scene->DestroyGameObject(creditsImageId);

        scene->DestroyGameObject(startButtonId);
        scene->DestroyGameObject(creditsButtonId);
        scene->DestroyGameObject(backButtonId);
        scene->DestroyGameObject(exitButtonId);

        scene->DestroyGameObject(creditsText1);
        scene->DestroyGameObject(creditsText2);
    }

    void SplashScreen::OnStartButtonPress()
    {
        EventManager::Emit<Frost::LoadLevelEvent>("assets/Scenes/Island/Island.bin");
        HideMenu();
    }

    void SplashScreen::OnExitButtonPress()
    {
        EventManager::Emit<Frost::WindowCloseEvent>();
    }

    void SplashScreen::OnBackButtonPress()
    {
        ShowMenu();
        HideCredits();
    }

    void SplashScreen::OnCreditsButtonPress()
    {
        ShowCredits();
    }

    void SplashScreen::ShowCredits()
    {
        HideMenu();
        creditsImageId.SetActive(true);
        creditsText1.SetActive(true);
        creditsText2.SetActive(true);
        backButtonId.SetActive(true);
    }
    void SplashScreen::HideCredits()
    {
        creditsImageId.SetActive(false);
        creditsText1.SetActive(false);
        creditsText2.SetActive(false);
        backButtonId.SetActive(false);
    }
    void SplashScreen::ShowMenu()
    {
        HideCredits();
        titleImageId.SetActive(true);
        startButtonId.SetActive(true);
        creditsButtonId.SetActive(true);
        exitButtonId.SetActive(true);
    }

    void SplashScreen::HideMenu()
    {
        titleImageId.SetActive(false);
        startButtonId.SetActive(false);
        creditsButtonId.SetActive(false);
        exitButtonId.SetActive(false);
    }
} // namespace GameLogic