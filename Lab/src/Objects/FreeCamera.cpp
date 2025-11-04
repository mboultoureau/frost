#include "FreeCamera.h"
#include "../Game.h"

FreeCamera::FreeCamera()
{
	Scene& _scene = Game::GetScene();

	_camera = _scene.CreateGameObject("FreeCamera");
	_scene.AddComponent<Frost::Transform>(_camera, Frost::Transform::Vector3{ 0.0f, 5.0f, 0.0f });
	_scene.AddComponent<Frost::WorldTransform>(_camera);
	_scene.AddComponent<Frost::Camera>(_camera);

	auto cameraComp = _scene.GetComponent<Frost::Camera>(_camera);
	cameraComp->backgroundColor[0] = 0.2f;
	cameraComp->backgroundColor[1] = 0.3f;
	cameraComp->backgroundColor[2] = 0.4f;
	cameraComp->backgroundColor[3] = 1.0f;
}
