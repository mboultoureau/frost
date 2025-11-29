#include "Sky.h"
#include "Frost.h"
#include "../../Game.h"

Sky::Sky()
{
	using namespace Frost;

	auto& scene = Game::GetScene();

	auto sky = scene.CreateGameObject("Sky");
	std::array<std::string, 6> faces = {
		"../Game/resources/textures/skybox/right.png",
		"../Game/resources/textures/skybox/left.png",
		"../Game/resources/textures/skybox/top.png",
		"../Game/resources/textures/skybox/bottom.png",
		"../Game/resources/textures/skybox/front.png",
		"../Game/resources/textures/skybox/back.png" };
	scene.AddComponent<Component::Skybox>(sky, faces);
}
