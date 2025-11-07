#include "MainLayer.h"
#include "Game.h"

using namespace Frost;


MainLayer::MainLayer() : Layer("MainLayer")
{

}

void MainLayer::OnAttach()
{
	Frost::Scene& _scene = Game::GetScene();

	_sphere = std::make_unique<Sphere>();
	_freeCamera = std::make_unique<FreeCamera>();
	_topCamera = std::make_unique<TopCamera>();
	_moto = std::make_unique<Moto>();
	//_ogre = std::make_unique<Ogre>();
	//_ogreNormal = std::make_unique<OgreNormal>();
	//_text = std::make_unique<Text>();
	//_plane = std::make_unique<Plane>();
	_hudLogo = std::make_unique<HUD_Logo>();
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
