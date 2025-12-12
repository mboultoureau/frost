#pragma once

#include <Frost.h>

namespace GameLogic
{
    class SplashScreen : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnDestroy() override;

    private:
        const std::string titlePath = "assets/HUD/game_title.png";
        Frost::GameObject titleImageId;
        const std::string creditsPath = "assets/HUD/credits.png";
        Frost::GameObject creditsImageId;

        const std::string idleStartPath = "assets/HUD/start-idle.png";
        const std::string hoverStartPath = "assets/HUD/start-hover.png";
        const std::string pressedStartPath = "assets/HUD/start-press.png";

        const std::string idleCreditsPath = "assets/HUD/credits-idle.png";
        const std::string hoverCreditsPath = "assets/HUD/credits-hover.png";
        const std::string pressedCreditsPath = "assets/HUD/credits-press.png";

        const std::string idleBackPath = "assets/HUD/back-idle.png";
        const std::string hoverBackPath = "assets/HUD/back-hover.png";
        const std::string pressedBackPath = "assets/HUD/back-press.png";

        Frost::GameObject creditsText1;
        Frost::GameObject creditsText2;
        Frost::GameObject creditsButtonId;
        Frost::GameObject startButtonId;
        Frost::GameObject backButtonId;

        void ShowCredits();
        void HideCredits();
        void ShowMenu();
        void HideMenu();
        void OnStartButtonPress();
        void OnCreditsButtonPress();
        void OnBackButtonPress();
    };
} // namespace GameLogic
