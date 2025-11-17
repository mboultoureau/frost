#include "TopCamera.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

TopCamera::TopCamera()
{
	Frost::Scene& _scene = Game::GetScene();

	_camera = _scene.CreateGameObject("Camera2");
	_scene.AddComponent<Transform>(
		_camera,
		Vector3{ 0.0f, 10.0f, 5.0f },
		EulerAngles{ 90.0_deg, 0.0f, 0.0f },
		Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<WorldTransform>(_camera, Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Camera>(_camera);

	auto cameraComp2 = _scene.GetComponent<Camera>(_camera);
	cameraComp2->projectionType = Camera::ProjectionType::Orthographic;
	cameraComp2->backgroundColor.r = 0.4f;
	cameraComp2->backgroundColor.g = 0.3f;
	cameraComp2->backgroundColor.b = 0.2f;
	cameraComp2->backgroundColor.a = 1.0f;
	cameraComp2->orthographicSize = 5.0f;
	cameraComp2->clearOnRender = true;
	cameraComp2->viewport = Viewport(0.05f, 0.05f, 0.2f, 0.2f);
}
