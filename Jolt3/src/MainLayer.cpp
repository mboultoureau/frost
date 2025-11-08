#include "MainLayer.h"
#include "Game.h"
#include "Objects/Cargo.h"
#include "Objects/Terrain.h"
#include "Objects/Bullet.h"
#include "Objects/Goal.h"
#include "Objects/LevelCamera.h"
#include "Frost/Core/Application.h"



using namespace Frost;

MainLayer::MainLayer() : Layer("MainLayer")
{
}

void MainLayer::OnAttach()
{
	Scene& _scene = Game::GetScene();
	Terrain terrain(_scene);

	_player = std::make_unique<Player>();
	//Goal goal;
	//Cargo cargo;
	//Bullet bullet{ Transform::Vector3{0.0f, 5.0f, 0.0f} };
	LevelCamera levelCamera;

	_pauseHandlerUUID = Application::Get().GetEventManager().Subscribe<Frost::PauseEvent>(
		FROST_BIND_EVENT_FN(MainLayer::OnGamePaused));

	_unpauseHandlerUUID = Application::Get().GetEventManager().Subscribe<Frost::UnPauseEvent>(
		FROST_BIND_EVENT_FN(MainLayer::OnGameUnpaused));
}

void MainLayer::OnUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();

	_scene.Update(deltaTime);
}

void MainLayer::OnLateUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();
	_scene.LateUpdate(deltaTime);
}

void MainLayer::OnFixedUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();

	_player->FixedUpdate(deltaTime);
	_scene.FixedUpdate(deltaTime);
}

void MainLayer::OnDetach()
{
	Application::Get().GetEventManager().Unsubscribe(EventType::GamePaused, _pauseHandlerUUID);
	Application::Get().GetEventManager().Unsubscribe(EventType::GameUnpaused, _unpauseHandlerUUID);
}

bool MainLayer::OnGamePaused(Frost::PauseEvent& e)
{
	_paused = true;
	return true;
}

bool MainLayer::OnGameUnpaused(Frost::UnPauseEvent& e)
{
	_paused = false;
	return true;
}
