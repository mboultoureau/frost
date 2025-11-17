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
	scene.AddComponent<Transform>(
		sun,
		Math::Vector3{ 0.0f, 100.0f, 0.0f },
		Math::EulerAngles{ 0.0f, 0.0f, -150.0_deg },
		Math::Vector3{ 1.0f, 1.0f, 1.0f }
	);

	scene.AddComponent<WorldTransform>(sun);

	// Light component
	scene.AddComponent<Light>(
		sun,
		LightType::Directional,
		Math::Vector3{ 1.0f, 1.0f, 1.0f },
		2.0f
	);
}
