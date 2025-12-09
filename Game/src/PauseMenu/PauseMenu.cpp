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

        pauseLogoId = scene.CreateGameObject("pause logo");
        resumeButtonId = scene.CreateGameObject("resume button");
        resetButtonId = scene.CreateGameObject("reset button");

        float buttonHeight = 0.25f;
        float buttonWidth = 0.2f;
        float logoHeight = 0.3f;
        float logoWidth = 0.2f;
        float centerbuttonX = 0.5f - (buttonWidth / 2.0f);
        float centerlogoX = 0.5f - (logoWidth / 2.0f);

        Viewport textViewport = { centerlogoX, 0.0f, buttonWidth, logoHeight };
        Viewport resumeViewport = { centerbuttonX, textViewport.y + logoHeight, buttonWidth, buttonHeight };
        Viewport resetViewport = { centerbuttonX, resumeViewport.y + buttonHeight, buttonWidth, buttonHeight };

        pauseLogoId.AddComponent<Component::HUDImage>(textViewport, pausePath, Material::FilterMode::POINT);

        resumeButtonId.AddComponent<Component::UIButton>(
            resumeViewport, idleResumePath, hoverResumePath, pressedResumePath, [this]() { OnUnpauseButtonPress(); });
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
        pauseLogoId.SetActive(true);
        resumeButtonId.SetActive(true);
        resetButtonId.SetActive(true);
    }

    void PauseMenu::HideMenu()
    {
        pauseLogoId.SetActive(false);
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
