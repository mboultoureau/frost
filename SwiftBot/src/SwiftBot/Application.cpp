#include "SwiftBot/Application.h"

#include <memory>

namespace Game
{
    Application::Application(Frost::ApplicationSpecification entryPoint) : Frost::Application(entryPoint) {}

    Application::~Application() {}

    void Application::OnApplicationReady()
    {
        // std::shared_ptr<Frost::Scene> mainMenuScene = Frost::SceneManager::LoadSceneFromFile(MAIN_MENU_PATH);
        std::shared_ptr<Frost::Scene> mainScene = Frost::SceneManager::LoadSceneFromFile(MAIN_SCENE_PATH);
        std::shared_ptr<Frost::Scene> hudScene = Frost::SceneManager::LoadSceneFromFile(HUD_SCENE_PATH);

        // Frost::Application::PushLayer<Frost::SceneLayer>(mainMenuScene, "MainMenuLayer");

        Frost::Application::PushLayer<Frost::SceneLayer>(mainScene, "GameLayer");
        Frost::Application::PushLayer<Frost::SceneLayer>(hudScene, "HUDLayer");

#ifdef FT_DEBUG
        auto debugLayer = Frost::Application::PushLayer<Frost::DebugLayer>();
        debugLayer->AddScene(mainScene.get());
// debugLayer->AddScene(hudScene.get());
#endif
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
    entryPoint.scriptPath = "bin/Debug-Windows-x64/SwiftBot/SwiftBotLogic.dll";

    return new Game::Application(entryPoint);
}