#include "Game.h"

#include "Frost/PauseMenu/PauseMenu.h"
#include "MainLayer.h"
#include "LabApp.h"
#include "Frost.h"

Game* Game::_singleton = nullptr;

Game::Game(Lab* app) : _app{ app }
{
	FT_ASSERT(_singleton == nullptr, "Game: Attempting to create a second instance of the singleton.");
	_singleton = this;

	InitGame();

	Application::Get().GetEventManager().Subscribe<Frost::ResetEvent>(
		FROST_BIND_EVENT_FN(Game::OnGameReset));
}

bool Game::OnGameReset(Frost::ResetEvent& e)
{
	_app->Reset();
	Scene* scene = _scene.release();
	delete scene;
	InitGame();
	return true;
}

void Game::InitGame()
{
	_scene = std::make_unique<Frost::Scene>("Scene");
	_app->PushLayer(new MainLayer());

	// TODO: Memory leak on destroy
	Frost::DebugLayer* debugLayer = new Frost::DebugLayer{};
	_app->PushLayer(debugLayer);

	Frost::PauseMenu* pauseLayer = new Frost::PauseMenu{};
	pauseLayer->AddScene(_scene.get());
	_app->PushLayer(pauseLayer);

	debugLayer->AddScene(_scene.get());

}

Game& Game::Get()
{
	FT_ASSERT(_singleton != nullptr, "Game: Get() called before Game was initialized. Ensure it is created in LabApp.");
	return *_singleton;
}
