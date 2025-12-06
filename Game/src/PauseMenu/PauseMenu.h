#pragma once

#include "Frost/Core/Layer.h"
#include "Frost/Utils/Math/NormalizedRect.h"
#include <Frost/Event/Events/Input/KeyPressedEvent.h>
#include <Frost/Scene/Components/UIButton.h>
#include <Frost/Scene/Scene.h>

namespace Frost
{
    class PauseMenu : public Layer
    {
    public:
        PauseMenu();

        static Layer::LayerName GetStaticName() { return "PauseMenuLayer"; }

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnLateUpdate(float deltaTime) override;

        void AddScene(Scene* scene) { _scenes.push_back(scene); }

    private:
        const std::string pausePath = "resources/textures/pause.png";
        GameObject pauseLogoId;
        NormalizedRect pauseNRect;

        const std::string idleResumePath = "resources/textures/resume-idle.png";
        const std::string hoverResumePath = "resources/textures/resume-hover.png";
        const std::string pressedResumePath = "resources/textures/resume-press.png";
        GameObject resumeButtonId;
        NormalizedRect resumeNRect;

        const std::string idleResetPath = "resources/textures/reload-idle.png";
        const std::string hoverResetPath = "resources/textures/reload-hover.png";
        const std::string pressedResetPath = "resources/textures/reload-press.png";
        GameObject resetButtonId;
        NormalizedRect resetNRect;

        void ProcessInput();
        void ShowMenu();
        void HideMenu();
        void OnUnpauseButtonPress();
        void OnResetButtonPress();
        bool _resetButtonReleased;
        bool _pauseButtonReleased;
        bool _gamePaused;
        std::vector<Scene*> _scenes;
    };
} // namespace Frost
