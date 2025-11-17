#include "Game.h"
#include "MainLayer.h"

#include "Frost/Debugging/DebugLayer.h"
#include "PauseMenu/PauseMenu.h"

Game* Game::_singleton = nullptr;

Game::Game(Lab* app) : _app{ app }
{
	FT_ASSERT(_singleton == nullptr && "Game: Attempting to create a second instance of the singleton.");
	_singleton = this;

	InitGame();

	EventManager::Subscribe<Frost::ResetEvent>(FROST_BIND_EVENT_FN(Game::OnGameReset));
}

Game& Game::Get()
{
	FT_ASSERT(_singleton != nullptr && "Game: Get() called before Game was initialized. Ensure it is created in LabApp.");
	return *_singleton;
}

bool Game::OnGameReset(Frost::ResetEvent& e)
{
	_app->Reset();
	_scene.reset();
	InitGame();
	return true;
}

void Game::InitGame()
{
	_scene = std::make_unique<Frost::Scene>("Scene");
	_app->PushLayer<MainLayer>();
	DebugLayer* debugLayer = _app->PushLayer<DebugLayer>();
	PauseMenu* pauseLayer = _app->PushLayer<PauseMenu>();
	
	//pauseLayer->AddScene(_scene.get());
	debugLayer->AddScene(_scene.get());
}
