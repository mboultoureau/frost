#include "PointLight.h"
#include "../Game.h"
#include "Frost.h"

using namespace Frost;

PointLight::PointLight()
{
    Scene& scene = Game::GetScene();

    auto light = scene.CreateGameObject("Point Light");

    light.AddComponent<Component::Light>(Component::LightPoint{});
    light.AddComponent<Component::Transform>(Math::Vector3{ 0.0f, 10.0f, 0.0f });
}
