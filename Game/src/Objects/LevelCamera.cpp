#include "LevelCamera.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

LevelCamera::LevelCamera()
{
	Frost::Scene& _scene = Game::GetScene();

	auto _camera = _scene.CreateGameObject("Level Camera");
	_scene.AddComponent<Transform>(
		_camera,
		Vector3{ 0.0f, 500.0f, -100.0f },
		EulerAngles{ 0.0_deg, 0.0_deg, 90.0_deg },
		Vector3{ 0.0f, 0.0f, 0.0f }
	);
	_scene.AddComponent<WorldTransform>(_camera, Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Camera>(_camera);

	auto cameraComp = _scene.GetComponent<Camera>(_camera);
	cameraComp->projectionType = Camera::ProjectionType::Orthographic;
	cameraComp->orthographicSize = 1000.0f;
	cameraComp->backgroundColor.r = 0.8f;
	cameraComp->backgroundColor.g = 0.8f;
	cameraComp->backgroundColor.b = 0.8f;
	cameraComp->backgroundColor.a = 1.0f;
	cameraComp->clearOnRender = true;
	cameraComp->viewport = Viewport(0.05f, 0.05f, 0.2f, 0.2f);
}
