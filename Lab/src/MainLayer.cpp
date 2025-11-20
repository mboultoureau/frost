#include "MainLayer.h"
#include "Game.h"
#include "Objects/BallRain.h"

using namespace Frost;


MainLayer::MainLayer() : Layer("MainLayer")
{

}

void MainLayer::OnAttach()
{
	Frost::Scene& _scene = Game::GetScene();

	_pointLight = std::make_unique<PointLight>();
	//_topCamera = std::make_unique<TopCamera>();

	//_sphere = std::make_unique<Sphere>();
	//_freeCamera = std::make_unique<FreeCamera>();
	_moto = std::make_unique<Moto>();
	_ogre = std::make_unique<Ogre>();
	_ogreNormal = std::make_unique<OgreNormal>();
	//_text = std::make_unique<Text>();
	//_plane = std::make_unique<Plane>();
	//_terrain = std::make_unique<Terrain>();
	_player = std::make_unique<Player>();
	_hierarchy = std::make_unique<HierarchyTest>();
	//auto _rain = std::make_unique<BallRain>();
	//_hudLogo = std::make_unique<HUD_Logo>();
	_sky = std::make_unique<Sky>();

	_pauseHandlerUUID = EventManager::Subscribe<Frost::PauseEvent>(
		FROST_BIND_EVENT_FN(MainLayer::OnGamePaused));

	_unpauseHandlerUUID = EventManager::Subscribe<Frost::UnPauseEvent>(
		FROST_BIND_EVENT_FN(MainLayer::OnGameUnpaused));
}

void MainLayer::OnUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();

	_scene.Update(deltaTime);
}

void MainLayer::OnFixedUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();

	_scene.FixedUpdate(deltaTime);
}

void MainLayer::OnLateUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();
	_scene.LateUpdate(deltaTime);
}

void MainLayer::OnDetach()
{
	//EventManager::Unsubscribe<EventType::GamePaused>(_pauseHandlerUUID);
	//EventManager::Unsubscribe<EventType::GameUnpaused>(_unpauseHandlerUUID);
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