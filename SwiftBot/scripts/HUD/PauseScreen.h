#pragma once

#include <Frost.h>

namespace GameLogic
{
    class PauseScreen : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnDestroy() override;
        void OnUpdate(float deltaTime) override;

    private:
        const std::string pausePath = "assets/HUD/pause.png";
        Frost::GameObject pauseLogoId;

        const std::string idleResumePath = "assets/HUD/resume-idle.png";
        const std::string hoverResumePath = "assets/HUD/resume-hover.png";
        const std::string pressedResumePath = "assets/HUD/resume-press.png";
        Frost::GameObject resumeButtonId;

        const std::string idleResetPath = "assets/HUD/reload-idle.png";
        const std::string hoverResetPath = "assets/HUD/reload-hover.png";
        const std::string pressedResetPath = "assets/HUD/reload-press.png";
        Frost::GameObject resetButtonId;

        const std::string idleMenuPath = "assets/HUD/menu-idle.png";
        const std::string hoverMenuPath = "assets/HUD/menu-hover.png";
        const std::string pressedMenuPath = "assets/HUD/menu-press.png";
        Frost::GameObject menuButtonId;

        void ProcessInput();
        void ShowMenu();
        void HideMenu();
        void OnUnpauseButtonPress();
        void OnResetButtonPress();
        void OnMenuButtonPress();

        bool _resetButtonReleased = true;
        bool _pauseButtonReleased = true;
        bool _gamePaused = false;
    };
} // namespace GameLogic