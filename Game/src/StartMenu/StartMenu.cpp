#include "StartMenu.h"
#include "../Game.h"
#include "Frost/Core/Application.h"
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"
#include "Frost/Scene/Components/HUDImage.h"
#include <Frost/Debugging/Logger.h>
#include <Frost/Renderer/Viewport.h>
#include <Frost/Asset/Font.h>
#include <Frost/Scene/Components/HUDText.h>
using namespace Frost::Component;

namespace Frost
{
    StartMenu::StartMenu() : Layer(GetStaticName()) {}

    void StartMenu::OnAttach()
    {
        auto& scene = Game::GetScene();

        titleImageId = scene.CreateGameObject("Start Title");
        creditsImageId = scene.CreateGameObject("Credits Title");
        startButtonId = scene.CreateGameObject("Start Button");
        creditsButtonId = scene.CreateGameObject("Credit Button");
        backButtonId = scene.CreateGameObject("Back Button");
        creditsText1 = scene.CreateGameObject("Text");
        creditsText2 = scene.CreateGameObject("Text");

        Viewport imageViewport;
        imageViewport.height = 1.0f;
        imageViewport.width = 0.85f;
        imageViewport.x = 0.075f;
        imageViewport.y = 0.0f;

        titleImageId.AddComponent<Component::HUDImage>(imageViewport, titlePath, Material::FilterMode::POINT);
        creditsImageId.AddComponent<Component::HUDImage>(imageViewport, creditsPath, Material::FilterMode::POINT);

        Viewport buttonPlayViewport;
        buttonPlayViewport.height = 0.25f;
        buttonPlayViewport.width = 0.2f;
        buttonPlayViewport.x = 0.4f;
        buttonPlayViewport.y = 0.75f;

        startButtonId.AddComponent<Component::UIButton>(
            buttonPlayViewport, idleStartPath, hoverStartPath, pressedStartPath, [this]() { OnStartButtonPress(); });

        auto& startButton = startButtonId.GetComponent<Component::UIButton>();
        startButton.buttonHitbox = buttonPlayViewport;

        Viewport buttonCreditsViewport;
        buttonCreditsViewport.height = 0.25f;
        buttonCreditsViewport.width = 0.18f;
        buttonCreditsViewport.x = 0.75f;
        buttonCreditsViewport.y = 0.08f;

        creditsButtonId.AddComponent<Component::UIButton>(buttonCreditsViewport,
                                                          idleCreditsPath,
                                                          hoverCreditsPath,
                                                          pressedCreditsPath,
                                                          [this]() { OnCreditsButtonPress(); });

        auto& creditsButton = creditsButtonId.GetComponent<Component::UIButton>();
        creditsButton.buttonHitbox = buttonCreditsViewport;

        Viewport buttonBackViewport;
        buttonBackViewport.height = 0.25f;
        buttonBackViewport.width = 0.18f;
        buttonBackViewport.x = 0.07f;
        buttonBackViewport.y = 0.08f;

        backButtonId.AddComponent<Component::UIButton>(
            buttonBackViewport, idleBackPath, hoverBackPath, pressedBackPath, [this]() { OnBackButtonPress(); });

        auto& backButton = backButtonId.GetComponent<Component::UIButton>();
        backButton.buttonHitbox = buttonBackViewport;

        auto font = AssetManager::LoadAsset<Font>("resources/fonts/ss-engine.regular.ttf");

        Viewport viewportText1;
        viewportText1.x = 0.4f;
        viewportText1.y = 0.85f;
        viewportText1.width = 0.5f;
        viewportText1.height = 0.5f;
        std::string initText1 = "ELIAS DEL POZO                            SIMON LE FLOCH";

        creditsText1.AddComponent<Frost::Component::HUDText>(viewportText1, font, initText1);

        Viewport viewportText2;
        viewportText2.x = 0.4f;
        viewportText2.y = 0.9f;
        viewportText2.width = 0.5f;
        viewportText2.height = 0.5f;
        std::string initText2 = "MATHIS BOULTOUREAU                   THOMAS VANWALLEGHEM";

        creditsText2.AddComponent<Frost::Component::HUDText>(viewportText2, font, initText2);

        EventManager::Emit<Frost::PauseEvent>();
        ShowMenu();
    }

    void StartMenu::OnDetach()
    {
        auto& scene = Game::GetScene();
        scene.DestroyGameObject(titleImageId);
        scene.DestroyGameObject(creditsImageId);

        scene.DestroyGameObject(startButtonId);
        scene.DestroyGameObject(creditsButtonId);
        scene.DestroyGameObject(backButtonId);

        scene.DestroyGameObject(creditsText1);
        scene.DestroyGameObject(creditsText2);
    }

    void StartMenu::OnUpdate(float deltaTime) {}

    void StartMenu::OnStartButtonPress()
    {
        EventManager::Emit<Frost::UnPauseEvent>();
        HideMenu();
    }

    void StartMenu::OnBackButtonPress()
    {
        ShowMenu();
        HideCredits();
    }

    void StartMenu::OnCreditsButtonPress()
    {
        ShowCredits();
    }

    void StartMenu::ShowCredits()
    {
        HideMenu();
        creditsImageId.SetActive(true);
        creditsText1.SetActive(true);
        creditsText2.SetActive(true);
        backButtonId.SetActive(true);
    }
    void StartMenu::HideCredits()
    {
        creditsImageId.SetActive(false);
        creditsText1.SetActive(false);
        creditsText2.SetActive(false);
        backButtonId.SetActive(false);
    }
    void StartMenu::ShowMenu()
    {
        HideCredits();
        titleImageId.SetActive(true);
        startButtonId.SetActive(true);
        creditsButtonId.SetActive(true);
    }

    void StartMenu::HideMenu()
    {
        titleImageId.SetActive(false);
        startButtonId.SetActive(false);
        creditsButtonId.SetActive(false);
    }
} // namespace Frost