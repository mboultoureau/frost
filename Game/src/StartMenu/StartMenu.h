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
        const std::string creditsPath = "resources/textures/credits.png";
        GameObject creditsImageId;

        const std::string idleStartPath = "resources/textures/start-idle.png";
        const std::string hoverStartPath = "resources/textures/start-hover.png";
        const std::string pressedStartPath = "resources/textures/start-press.png";

        const std::string idleCreditsPath = "resources/textures/credits-idle.png";
        const std::string hoverCreditsPath = "resources/textures/credits-hover.png";
        const std::string pressedCreditsPath = "resources/textures/credits-press.png";

        const std::string idleBackPath = "resources/textures/back-idle.png";
        const std::string hoverBackPath = "resources/textures/back-hover.png";
        const std::string pressedBackPath = "resources/textures/back-press.png";

        GameObject creditsText1;
        GameObject creditsText2;
        GameObject creditsButtonId;
        GameObject startButtonId;
        GameObject backButtonId;

        void ShowCredits();
        void HideCredits();
        void ShowMenu();
        void HideMenu();
        void OnStartButtonPress();
        void OnCreditsButtonPress();
        void OnBackButtonPress();
    };
} // namespace Frost