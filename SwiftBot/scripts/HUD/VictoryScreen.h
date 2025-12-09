#pragma once

#include <Frost.h>

namespace GameLogic
{
    class VictoryScreen : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnDestroy() override;

    private:
        const std::string WIN_PATH = "assets/HUD/victoryScreen.png";
        Frost::GameObject victoryImageId;

        const std::string hoverPath = "assets/HUD/reload-hover.png";
        const std::string idlePath = "assets/HUD/reload-idle.png";
        const std::string pressPath = "assets/HUD/reload-press.png";
        Frost::GameObject restartButtonId;

        void OnRestartButtonPress();
    };
} // namespace GameLogic