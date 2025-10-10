#include "MainLayer.h"

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Renderer/RendererAPI.h"

MainLayer::MainLayer() : Layer("MainLayer")
{
	_scene = std::make_unique<Frost::Scene>();

	auto sphere = _scene->CreateGameObject("Sphere");
	_scene->AddComponent<Frost::Transform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene->AddComponent<Frost::WorldTransform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene->AddComponent<Frost::MeshRenderer>(sphere, "Cube.fbx");
	
	auto camera = _scene->CreateGameObject("Camera");
	_scene->AddComponent<Frost::Transform>(camera, Frost::Transform::Vector3{ 0.0f, 0.0f, -5.0f });
	_scene->AddComponent<Frost::WorldTransform>(camera, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene->AddComponent<Frost::Camera>(camera);
}

void MainLayer::OnUpdate(float deltaTime)
{
}
