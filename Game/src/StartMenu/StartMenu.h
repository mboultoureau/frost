#pragma once

#include "Frost/Core/Layer.h"
#include <Frost/Scene/Components/UIButton.h>
#include <Frost/Scene/Scene.h>

namespace Frost
{
    class StartMenu : public Layer
    {
    public:
        StartMenu();

        static Layer::LayerName GetStaticName() { return "StartMenuLayer"; }

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;

    private:
        const std::string titlePath = "resources/textures/game-title.png";
        GameObject titleImageId;

        const std::string idleStartPath = "resources/textures/start-idle.png";
        const std::string hoverStartPath = "resources/textures/start-hover.png";
        const std::string pressedStartPath = "resources/textures/start-press.png";
        GameObject startButtonId;

        void ShowMenu();
        void HideMenu();
        void OnStartButtonPress();
    };
} // namespace Frost