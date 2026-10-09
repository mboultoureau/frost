#include "SwiftBot/Application.h"
#include "Physics/GeneratedPhysicsLayers.h"

#include <memory>

namespace Game
{
    Application::Application(Frost::ApplicationSpecification entryPoint) : Frost::Application(entryPoint)
    {
        ConfigurePhysics({ .broadPhaseLayerInterface = new GameBroadPhaseLayerInterface(),
                           .objectLayerPairFilter = new GameObjectLayerPairFilter(),
                           .objectVsBroadPhaseLayerFilter = new GameObjectVsBroadPhaseLayerFilter() });
    }

    Application::~Application()
    {
        Frost::EventManager::Unsubscribe<Frost::LoadLevelEvent>(_loadLevelEventHandlerId);
        Frost::EventManager::Unsubscribe<Frost::ResetEvent>(_resetEventHandlerId);
    }

    void Application::OnApplicationReady()
    {
        Frost::EventManager::Subscribe<Frost::ResetEvent>(
            [this](Frost::ResetEvent& e)
            {
                Frost::FT_INFO("ResetEvent received. Resetting application...");
                this->RestartLevel();

                return true;
            });

        Frost::EventManager::Subscribe<Frost::LoadLevelEvent>(
            [this](Frost::LoadLevelEvent& e)
            {
                Frost::FT_INFO("LoadLevelEvent received. Loading level: {}", e.GetLevelName());
                this->LoadLevel(e.GetLevelName());
                return true;
            });

        std::string scenePath = RAINBOW_ROAD_SCENE_PATH;
        if (!std::filesystem::exists(scenePath) && std::filesystem::exists("SwiftBot/" + scenePath))
        {
            scenePath = "SwiftBot/" + scenePath;
        }

        LoadLevel(scenePath);

        Frost::FT_INFO("Application is ready.");
    }

    void Application::LoadLevel(const std::string& levelPath)
    {
        _Clean();

        std::string resolvedPath = levelPath;
        if (!std::filesystem::exists(resolvedPath) && std::filesystem::exists("SwiftBot/" + resolvedPath))
        {
            resolvedPath = "SwiftBot/" + resolvedPath;
        }

        Frost::FT_INFO("Loading level from path: {}", resolvedPath);

        std::shared_ptr<Frost::Scene> levelScene = Frost::SceneManager::LoadSceneFromFile(resolvedPath);
        if (levelScene)
        {
            Frost::Application::PushLayer<Frost::SceneLayer>(levelScene, "LevelLayer");

            _currentLevelPath = resolvedPath;

            auto debugLayer = Frost::Application::PushLayer<Frost::DebugLayer>();
            debugLayer->AddScene(levelScene.get());
        }
    }

    void Application::RestartLevel()
    {
        Frost::FT_INFO("Restarting level...");

        _Clean();

        if (!_currentLevelPath.empty())
        {
            LoadLevel(_currentLevelPath);
        }
        else
        {
            ShowMainMenu();
        }
    }

    void Application::ShowMainMenu()
    {
        Frost::FT_INFO("Returning to main menu...");

        _Clean();

        std::string scenePath = MAIN_MENU_SCENE_PATH;
        if (!std::filesystem::exists(scenePath) && std::filesystem::exists("SwiftBot/" + scenePath))
        {
            scenePath = "SwiftBot/" + scenePath;
        }

        std::shared_ptr<Frost::Scene> mainMenuScene = Frost::SceneManager::LoadSceneFromFile(scenePath);
        if (mainMenuScene)
        {
            Frost::Application::PushLayer<Frost::SceneLayer>(mainMenuScene, "MainMenuLayer");

            auto* debugLayer = Frost::Application::PushLayer<Frost::DebugLayer>();
            debugLayer->AddScene(mainMenuScene.get());
        }
    }

    void Application::_Clean()
    {
        this->Reset();
        Frost::SceneManager::Shutdown();
    }
} // namespace Game

Frost::Application*
Frost::CreateApplication(ApplicationSpecification entryPoint)
{
    entryPoint.title = L"SwiftBot";
    entryPoint.windowWidth = 1280;
    entryPoint.windowHeight = 720;
    entryPoint.iconPath = "assets/icons/game_icon.ico";
    entryPoint.consoleIconPath = "assets/icons/console_icon.ico";
    entryPoint.scriptPath = "SwiftBotLogic.dll";

    return new Game::Application(entryPoint);
}