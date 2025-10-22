#include "MainLayer.h"
#include "Game.h"

#include "Frost/Scene/Components/Script.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Utils/Math/Angle.h"

#include <vector>
#include <array>
#include <d3d11.h>

using namespace Frost;

class Log1 : public Frost::Script
{
public:
	void OnUpdate(float deltaTime) override
	{
		auto camera = GetECS()->GetComponent<Frost::Camera>(GetGameObject());
		camera->backgroundColor[0] += 0.01f * deltaTime;
	}
};

class Log2 : public Frost::Script
{
public:
	void OnFixedUpdate(float deltaTime) override
	{
		auto transform = GetECS()->GetComponent<Frost::Transform>(GetGameObject());
		transform->position.x += 5.0f * deltaTime;
	}
};

MainLayer::MainLayer() : Layer("MainLayer")
{
	Log1 log1;
	Log2 log2;

	Frost::Scene& _scene = Game::GetScene();

	auto sphere = _scene.CreateGameObject("Sphere");
	_scene.AddComponent<Frost::Transform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::WorldTransform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::MeshRenderer>(sphere, "./resources/meshes/Sphere.fbx");
	_scene.AddScript<Log2>(sphere);

	auto sphere2 = _scene.CreateGameObject("Sphere2");
	_scene.AddComponent<Frost::Transform>(sphere2, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::WorldTransform>(sphere2, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::MeshRenderer>(sphere2, "./resources/meshes/Sphere.fbx");

	auto camera = _scene.CreateGameObject("Camera");
	_scene.AddComponent<Frost::Transform>(camera, Frost::Transform::Vector3{ 0.0f, 0.0f, -10.0f });
	_scene.AddComponent<Frost::WorldTransform>(camera, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::Camera>(camera);
	_scene.AddScript<Log1>(camera);

	auto cameraComp = _scene.GetComponent<Frost::Camera>(camera);
	cameraComp->backgroundColor[0] = 0.2f;
	cameraComp->backgroundColor[1] = 0.3f;
	cameraComp->backgroundColor[2] = 0.4f;
	cameraComp->backgroundColor[3] = 1.0f;

	auto camera2 = _scene.CreateGameObject("Camera2");
	_scene.AddComponent<Frost::Transform>(camera2, Frost::Transform::Vector3{ 0.0f, 0.0f, -10.0f });
	_scene.AddComponent<Frost::WorldTransform>(camera2, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::Camera>(camera2);

	auto camera2Transform = _scene.GetComponent<Frost::Transform>(camera2);
	camera2Transform->rotation.z = Frost::Angle<Frost::Degree>(90.0_deg).value();

	auto cameraComp2 = _scene.GetComponent<Frost::Camera>(camera2);
	cameraComp2->projectionType = Frost::Camera::ProjectionType::Orthographic;
	cameraComp2->backgroundColor[0] = 0.4f;
	cameraComp2->backgroundColor[1] = 0.3f;
	cameraComp2->backgroundColor[2] = 0.2f;
	cameraComp2->backgroundColor[3] = 1.0f;
	cameraComp2->clearOnRender = true;
	cameraComp2->viewport = Frost::Viewport(0.05f, 0.05f, 0.2f, 0.2f);
}

void MainLayer::OnUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();

	_scene.Update(deltaTime);
}

void MainLayer::OnFixedUpdate(float deltaTime)
{
	Frost::Scene& _scene = Game::GetScene();

	_scene.FixedUpdate(deltaTime);
}
