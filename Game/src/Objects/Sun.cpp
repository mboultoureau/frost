#include "Sun.h"
#include "../Game.h"
#include "Frost.h"

using namespace Frost;
using namespace Frost::Component;
using namespace Frost::Math;

Sun::Sun()
{
    Scene& scene = Game::GetScene();

    auto sun = scene.CreateGameObject("Sun");

    // Transform component
    sun.AddComponent<Transform>(Math::Vector3{ 0.0f, 500.0f, 0.0f },
                                Math::EulerAngles{ 0.0f, 0.0f, 140.0_deg },
                                Math::Vector3{ 1.0f, 1.0f, 1.0f });

    sun.AddComponent<WorldTransform>();

    // Light component
    auto& light = sun.AddComponent<Light>(LightDirectional{});
    light.color = Math::Color3{ 1.0f, 1.0f, 1.0f };
    light.intensity = 2.0f;
}
