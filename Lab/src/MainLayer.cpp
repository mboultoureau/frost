#include "MainLayer.h"

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Shader.h"

#include <vector>
#include <array>
#include <d3d11.h>

MainLayer::MainLayer() : Layer("MainLayer")
{
	_scene = std::make_unique<Frost::Scene>();

	auto sphere = _scene->CreateGameObject("Sphere");
	_scene->AddComponent<Frost::Transform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene->AddComponent<Frost::WorldTransform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene->AddComponent<Frost::MeshRenderer>(sphere, "Sphere.fbx");
	
	auto camera = _scene->CreateGameObject("Camera");
	_scene->AddComponent<Frost::Transform>(camera, Frost::Transform::Vector3{ -5.0f, 0.0f, -5.0f });
	_scene->AddComponent<Frost::WorldTransform>(camera, Frost::Transform::Vector3{ 0.0f, 0.0f, -5.0f });
	_scene->AddComponent<Frost::Camera>(camera);

	// Modify the color of the camera component
	auto cameraComp = _scene->GetComponent<Frost::Camera>(camera);
	cameraComp->backgroundColor[0] = 0.2f;
	cameraComp->backgroundColor[1] = 0.3f;
	cameraComp->backgroundColor[2] = 0.4f;
	cameraComp->backgroundColor[3] = 1.0f;
}

void MainLayer::OnUpdate(float deltaTime)
{
	_scene->Update(deltaTime);
}
