#pragma once

#include "Frost.h"
#include "Frost/Core/EntryPoint.h"

#include <string>

namespace Game
{
    class Application : public Frost::Application
    {
    public:
        Application(Frost::ApplicationSpecification entryPoint);
        ~Application();

        void OnApplicationReady() override;

        void LoadLevel(const std::string& levelPath);
        void RestartLevel();
        void ShowMainMenu();

    private:
        void _Clean();

        std::string _currentLevelPath;

        constexpr static const char* MAIN_MENU_PATH = "assets/Scenes/MainMenu/MainMenu.bin";
        constexpr static const char* MAIN_SCENE_PATH = "assets/Scenes/Island/Island.bin";
        constexpr static const char* HUD_SCENE_PATH = "assets/Scenes/HUD/HUD.bin";
        constexpr static const char* MAIN_MENU_SCENE_PATH = "assets/Scenes/MainMenu/MainMenu.bin";

        Frost::EventHandlerId _loadLevelEventHandlerId;
        Frost::EventHandlerId _resetEventHandlerId;
    };
} // namespace Game
