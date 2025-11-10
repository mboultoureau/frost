#include "Game.h"

#include "MainLayer.h"

#include "Frost/Debugging/DebugLayer.h"

#include "Frost/PauseMenu/PauseMenu.h"

#include <cassert>

Game* Game::_singleton = nullptr;

Game::Game(Lab* app) : _app{ app }
{
	assert(_singleton == nullptr && "Game: Attempting to create a second instance of the singleton.");
	_singleton = this;

	InitGame();

	Application::Get().GetEventManager().Subscribe<Frost::ResetEvent>(
		FROST_BIND_EVENT_FN(Game::OnGameReset));
}

Game& Game::Get()
{
	assert(_singleton != nullptr && "Game: Get() called before Game was initialized. Ensure it is created in LabApp.");
	return *_singleton;
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
