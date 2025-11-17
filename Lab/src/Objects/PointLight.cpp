#include "PointLight.h"
#include "../Game.h"
#include "Frost.h"

using namespace Frost;

PointLight::PointLight()
{
	Scene& scene = Game::GetScene();
	
	auto light = scene.CreateGameObject("Point Light");
	
	scene.AddComponent<Component::Light>(light);
	auto lightComp = scene.GetComponent<Component::Light>(light);
	lightComp->type = Component::LightType::Directional;

	scene.AddComponent<Component::Transform>(
		light,
		Math::Vector3{ 0.0f, 10.0f, 0.0f },
		Math::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f },
		Math::Vector3{ 1.0f, 1.0f, 1.0f }
	);

	scene.AddComponent<Component::WorldTransform>(light);
}
