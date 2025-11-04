#include "TopCamera.h"
#include "../Game.h"

TopCamera::TopCamera()
{
	Frost::Scene& _scene = Game::GetScene();

	_camera = _scene.CreateGameObject("Camera2");
	_scene.AddComponent<Frost::Transform>(
		_camera,
		Frost::Transform::Vector3{ 0.0f, 10.0f, 5.0f },
		Frost::Transform::Vector3{ angle_traits<Degree>::to_neutral(90.0), 0.0f, 0.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f }
	);
	_scene.AddComponent<Frost::WorldTransform>(_camera, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::Camera>(_camera);

	auto cameraComp2 = _scene.GetComponent<Frost::Camera>(_camera);
	cameraComp2->projectionType = Frost::Camera::ProjectionType::Orthographic;
	cameraComp2->backgroundColor[0] = 0.4f;
	cameraComp2->backgroundColor[1] = 0.3f;
	cameraComp2->backgroundColor[2] = 0.2f;
	cameraComp2->backgroundColor[3] = 1.0f;
	cameraComp2->orthographicSize = 5.0f;
	cameraComp2->clearOnRender = true;
	cameraComp2->viewport = Frost::Viewport(0.05f, 0.05f, 0.2f, 0.2f);
}
