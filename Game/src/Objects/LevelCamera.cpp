#include "LevelCamera.h"
#include "../Game.h"

LevelCamera::LevelCamera()
{
	Frost::Scene& _scene = Game::GetScene();

	auto _camera = _scene.CreateGameObject("Level Camera");
	_scene.AddComponent<Frost::Transform>(
		_camera,
		Frost::Transform::Vector3{ -380, 100.0f, -130 },
		Frost::Transform::Vector3{ 0, 1.6f, 0.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f }
	);
	_scene.AddComponent<Frost::WorldTransform>(_camera, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::Camera>(_camera);

	auto cameraComp = _scene.GetComponent<Frost::Camera>(_camera);
	cameraComp->projectionType = Frost::Camera::ProjectionType::Orthographic;
	cameraComp->backgroundColor[0] = 0.8f;
	cameraComp->backgroundColor[1] = 0.8f;
	cameraComp->backgroundColor[2] = 0.8f;
	cameraComp->backgroundColor[3] = 1.0f;
	cameraComp->orthographicSize = 75.0f;
	cameraComp->clearOnRender = true;
	cameraComp->viewport = Frost::Viewport(0.05f, 0.05f, 0.2f, 0.2f);
}
