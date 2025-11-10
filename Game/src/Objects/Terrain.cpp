#include "Terrain.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"
#include <Frost/Renderer/TextureLibrary.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>


Terrain::Terrain()
{
	using namespace JPH;
	Scene& _scene = Game::GetScene();


	// Road
	_terrain = _scene.CreateGameObject("Terrain");
	_scene.AddComponent<Frost::Transform>(
		_terrain,
		Frost::Transform::Vector3{ 0.0f, -10.0f, -150.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Frost::Transform::Vector3{ 1000.0f, 100.0f, 1000.0f }
	);
	heightScale = 1.0f;
	filepath = "./resources/textures/Map_RainbowRoad.png";
	_scene.AddComponent<Frost::WorldTransform>(_terrain, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });

	auto material = Material();
	material.diffuseColor = DirectX::XMFLOAT3{ 1, 1, 1 };
	material.roughnessValue = 0;
//	material.emissiveColor = DirectX::XMFLOAT3{ 0.5, 0.5, 0.5 };
	material.uvTiling = { 100,100 };
	material.filterMode = Material::FilterMode::LINEAR;
	material.diffuseTextures.push_back(TextureLibrary::Get().GetTexture("./resources/textures/rainbow.jpg", TextureType::DIFFUSE));
	_scene.AddComponent<Frost::ModelRenderer>(_terrain, IsHeightMapRenderer(), filepath, material, TextureChannel::R, 50, heightScale);

	auto scale = _scene.GetComponent<Transform>(_terrain)->scale;
	auto texture = static_cast<TextureDX11*>(Frost::TextureLibrary::Get().GetTexture(filepath, TextureType::UNKNOWN).get());
	ShapeRefC heightFieldShape = Physics::Get().CreateHeightFieldShapeFromTexture(texture, TextureChannel::R, heightScale, scale);
	//ShapeRefC heightFieldShape = BoxShapeSettings(Vec3(1000.0f, 20.0f, 1000.0f)).Create().Get();
	BodyCreationSettings bodySettings(
		heightFieldShape,
		RVec3(0, -10, -150),
		Quat::sIdentity(),
		EMotionType::Static,
		ObjectLayers::NON_MOVING
	);
	bodySettings.mRestitution = 0.0f;

	_scene.AddComponent<Frost::RigidBody>(_terrain, bodySettings, _terrain, JPH::EActivation::Activate);




	// Ground 
	auto ground = _scene.CreateGameObject("Ground");
	_scene.AddComponent<Frost::Transform>(
		ground,
		Frost::Transform::Vector3{ 0.0f, -10.0f, -150.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Frost::Transform::Vector3{ 1000.0f, 100.0f, 1000.0f }
	);
	heightScale = 1.0f;
	filepath = "./resources/textures/Map_RainbowRoad.png";
	_scene.AddComponent<Frost::WorldTransform>(ground, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });

	auto material2 = Material();
	material2.diffuseColor = DirectX::XMFLOAT3{ 0.5, 0.5, 1 };
	material2.roughnessValue = 0;
	material2.emissiveColor = DirectX::XMFLOAT3{ 0.1, 0.1, 0.5 };
	material2.uvTiling = { 10,10 };
	material2.filterMode = Material::FilterMode::LINEAR;
	material2.diffuseTextures.push_back(TextureLibrary::Get().GetTexture("./resources/textures/dirt.jpg", TextureType::DIFFUSE));
	_scene.AddComponent<Frost::ModelRenderer>(ground, IsHeightMapRenderer(), filepath, material2, TextureChannel::G, 50, heightScale);
}
