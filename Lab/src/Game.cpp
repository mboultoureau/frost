#include "Game.h"

#include "MainLayer.h"
#include "LabApp.h"
#include "Frost.h"

Game* Game::_singleton = nullptr;

Game::Game(Lab* app) : _app{ app }
{
	FT_ASSERT(_singleton == nullptr, "Game: Attempting to create a second instance of the singleton.");
	_singleton = this;

	_scene = std::make_unique<Frost::Scene>("Scene");
	_app->PushLayer(new MainLayer());
	
	// Memory leak
	Frost::DebugLayer* debugLayer = new Frost::DebugLayer{};
	_app->PushLayer(debugLayer);
	debugLayer->AddScene(_scene.get());
}

Game& Game::Get()
{
	FT_ASSERT(_singleton != nullptr, "Game: Get() called before Game was initialized. Ensure it is created in LabApp.");
	return *_singleton;
}
