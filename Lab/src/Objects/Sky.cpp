#include "Sky.h"
#include "../Game.h"
#include "Frost.h"


Sky::Sky()
{
	using namespace Frost;

	auto& scene = Game::GetScene();

	auto sky = scene.CreateGameObject("Sky");
	std::array<std::string, 6> faces = {
		"../Lab/resources/textures/skybox/right.jpg",
		"../Lab/resources/textures/skybox/left.jpg",
		"../Lab/resources/textures/skybox/top.jpg",
		"../Lab/resources/textures/skybox/bottom.jpg",
		"../Lab/resources/textures/skybox/front.jpg",
		"../Lab/resources/textures/skybox/back.jpg"
	};
	scene.AddComponent<Component::Skybox>(sky, faces);
}
