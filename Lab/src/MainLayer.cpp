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
	_scene->AddComponent<Frost::Transform>(camera, Frost::Transform::Vector3{ 0.0f, 0.0f, -5.0f });
	_scene->AddComponent<Frost::WorldTransform>(camera, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene->AddComponent<Frost::Camera>(camera);

	std::vector<int> vertices = {
		0, 0, 0,
		1, 0, 0,
		1, 1, 0,
		0, 1, 0
	};

	Frost::VertexBuffer vb;
	vb.Create(vertices.data(), sizeof(vertices));

	std::vector<int> indices = {
		0, 1, 2,
		2, 3, 0
	};

	Frost::IndexBuffer ib;
	ib.Create(indices.data(), sizeof(indices));

	struct Constant
	{
		float color[4];
	};

	Frost::ConstantBuffer cb;
	Constant constantData = { {1.0f, 0.0f, 0.0f, 1.0f} };
	cb.Create(&constantData, sizeof(Constant));

	std::array<D3D11_INPUT_ELEMENT_DESC, 3> layout = {
		D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	Frost::VertexShader vs;
	vs.Create(L"resources/shaders/Vertex.hlsl", layout.data(), static_cast<UINT>(layout.size()));

	Frost::PixelShader ps;
	ps.Create(L"resources/shaders/Pixel.hlsl");


}

void MainLayer::OnUpdate(float deltaTime)
{
	_scene->Update(deltaTime);
}
