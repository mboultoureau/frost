#include "TV.h"
#include "../Game.h"

TV::TV()
{
	auto& scene = Game::GetScene();
	gameObject = scene.CreateGameObject("TV");
}
