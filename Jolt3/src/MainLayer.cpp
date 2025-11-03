#include "MainLayer.h"
#include "Game.h"
#include "Objects/Cargo.h"
#include "Objects/Terrain.h"
#include "Objects/Bullet.h"
#include "Objects/Goal.h"
#include "Objects/LevelCamera.h"

using namespace Frost;

MainLayer::MainLayer() : Layer("MainLayer")
{
}

void MainLayer::OnAttach()
{
	Scene& _scene = Game::GetScene();
	Terrain terrain(_scene);

	_player = std::make_unique<Player>();
	Goal goal;
	Cargo cargo;
	Bullet bullet{ Transform::Vector3{0.0f, 5.0f, 0.0f} };
	LevelCamera levelCamera;
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
