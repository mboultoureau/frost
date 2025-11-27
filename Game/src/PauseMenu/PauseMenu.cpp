#include "PauseMenu.h"
#include "../Game.h"
#include "Frost/Core/Application.h"
#include "Frost/Event/Events/PauseMenu/PauseEvent.h"
#include "Frost/Event/Events/PauseMenu/ResetEvent.h"
#include "Frost/Event/Events/PauseMenu/UnPauseEvent.h"
#include "Frost/Input/Devices/Keyboard.h"
#include "Frost/Input/Input.h"
#include <Frost/Debugging/Logger.h>

using namespace Frost::Component;

namespace Frost
{
    PauseMenu::PauseMenu() :
        Layer(GetStaticName()), _resetButtonReleased(false), _pauseButtonReleased(true), _gamePaused(false)
    {
    }

    void PauseMenu::OnAttach()
    {
        auto& scene = Game::GetScene();

        pauseTextId = scene.CreateGameObject("pause text");
        resumeButtonId = scene.CreateGameObject("resume button");
        resetButtonId = scene.CreateGameObject("reset button");

        float buttonWidth = 0.25f;
        float buttonHeight = 0.08f;
        float textHeight = 0.1f;
        float centerX = 0.5f - (buttonWidth / 2.0f);
        float gap = 0.02f;

        Viewport textViewport = { centerX, 0.3f, buttonWidth, textHeight };
        Viewport resumeViewport = { centerX, textViewport.y + textHeight + gap, buttonWidth, buttonHeight };
        Viewport resetViewport = { centerX, resumeViewport.y + buttonHeight + gap, buttonWidth, buttonHeight };

        pauseTextId.AddComponent<Component::HUDImage>(textViewport, pausePath, Material::FilterMode::POINT);

        resumeButtonId.AddComponent<Component::UIButton>(
            resumeViewport, idleResumePath, hoverResumePath, hoverResumePath, [this]() { OnUnpauseButtonPress(); });
        auto& resumeButton = resumeButtonId.GetComponent<Component::UIButton>();
        resumeButton.buttonHitbox = resumeViewport;
        resetButtonId.AddComponent<Component::UIButton>(
            resetViewport, idleResetPath, hoverResetPath, hoverResetPath, [this]() { OnResetButtonPress(); });
        auto& resetButton = resetButtonId.GetComponent<Component::UIButton>();
        resetButton.buttonHitbox = resetViewport;
        HideMenu();
    }

    void PauseMenu::OnDetach() {}

    void PauseMenu::OnUpdate(float deltaTime)
    {
        ProcessInput();
    }

    void PauseMenu::OnLateUpdate(float deltaTime) {}

    void PauseMenu::ProcessInput()
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

    void PauseMenu::ShowMenu()
    {
        pauseTextId.SetActive(true);
        resumeButtonId.SetActive(true);
        resetButtonId.SetActive(true);
    }

    void PauseMenu::HideMenu()
    {
        pauseTextId.SetActive(false);
        resumeButtonId.SetActive(false);
        resetButtonId.SetActive(false);
    }

    void PauseMenu::OnUnpauseButtonPress()
    {
        EventManager::Emit<Frost::UnPauseEvent>();
        HideMenu();
        _gamePaused = false;
    }

    void PauseMenu::OnResetButtonPress()
    {
        EventManager::Emit<Frost::ResetEvent>();
        _resetButtonReleased = false;
    }
} // namespace Frost
