#pragma once

#include "Frost.h"
#include "Frost/Core/EntryPoint.h"

namespace Game
{
    class Application : public Frost::Application
    {
    public:
        Application(Frost::ApplicationSpecification entryPoint);
        ~Application();

        void OnApplicationReady() override;

    private:
        constexpr static const char* MAIN_MENU_PATH = "assets/Scenes/MainMenu/MainMenu.bin";
        constexpr static const char* MAIN_SCENE_PATH = "assets/Scenes/RainbowRoad/RainbowRoad.bin";
        constexpr static const char* HUD_SCENE_PATH = "assets/Scenes/HUD/HUD.bin";
    };
} // namespace Game
