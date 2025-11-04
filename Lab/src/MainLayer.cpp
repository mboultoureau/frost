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
	_plane = std::make_unique<Plane>();
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
