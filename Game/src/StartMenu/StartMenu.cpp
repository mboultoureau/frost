#include "StartMenu.h"
#include "../Game.h"
#include "Frost/Core/Application.h"
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"
#include "Frost/Scene/Components/HUDImage.h"
#include <Frost/Debugging/Logger.h>
#include <Frost/Renderer/Viewport.h>

using namespace Frost::Component;

namespace Frost
{
    StartMenu::StartMenu() : Layer(GetStaticName()) {}

    void StartMenu::OnAttach()
    {
        auto& scene = Game::GetScene();

        titleImageId = scene.CreateGameObject("Start Title");
        startButtonId = scene.CreateGameObject("Start Button");

        Viewport imageViewport;
        imageViewport.height = 0.7f;
        imageViewport.width = 0.6f;
        imageViewport.x = 0.2f;
        imageViewport.y = 0.1f;

        Viewport buttonViewport;
        buttonViewport.height = 0.2f;
        buttonViewport.width = 0.2f;
        buttonViewport.x = 0.4f;
        buttonViewport.y = 0.8f;

        titleImageId.AddComponent<Component::HUDImage>(imageViewport, titlePath, Material::FilterMode::POINT);

        startButtonId.AddComponent<Component::UIButton>(
            buttonViewport, idleStartPath, hoverStartPath, pressedStartPath, [this]() { OnStartButtonPress(); });

        auto& startButton = startButtonId.GetComponent<Component::UIButton>();
        startButton.buttonHitbox = buttonViewport;

        EventManager::Emit<Frost::PauseEvent>();
        ShowMenu();
    }

    void StartMenu::OnDetach()
    {
        auto& scene = Game::GetScene();
        scene.DestroyGameObject(titleImageId);
        scene.DestroyGameObject(startButtonId);
    }

    void StartMenu::OnUpdate(float deltaTime) {}

    void StartMenu::OnStartButtonPress()
    {
        EventManager::Emit<Frost::UnPauseEvent>();
        HideMenu();
    }

    void StartMenu::ShowMenu()
    {
        titleImageId.SetActive(true);
        startButtonId.SetActive(true);
    }

    void StartMenu::HideMenu()
    {
        titleImageId.SetActive(false);
        startButtonId.SetActive(false);
    }
} // namespace Frost