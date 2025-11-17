#include "FreeCamera.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Component;

FreeCamera::FreeCamera()
{
	Scene& _scene = Game::GetScene();

	_camera = _scene.CreateGameObject("FreeCamera");
	_scene.AddComponent<Transform>(_camera, Math::Vector3{ 0.0f, 5.0f, 0.0f });
	_scene.AddComponent<WorldTransform>(_camera);
	_scene.AddComponent<Camera>(_camera);
	_scene.AddComponent<Light>(_camera);

	auto cameraComp = _scene.GetComponent<Camera>(_camera);
	cameraComp->backgroundColor.r = 0.2f;
	cameraComp->backgroundColor.g = 0.3f;
	cameraComp->backgroundColor.b = 0.4f;
	cameraComp->backgroundColor.a = 1.0f;
}
