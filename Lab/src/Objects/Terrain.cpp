#include "Terrain.h"
#include "../Game.h"
#include <Frost/Renderer/TextureLibrary.h>
#include "../Physics/PhysicsLayer.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

Terrain::Terrain()
{
	using namespace JPH;
	Scene& _scene = Game::GetScene();

	_terrain = _scene.CreateGameObject("Terrain");
	_scene.AddComponent<Frost::Transform>(
		_terrain,
		Frost::Transform::Vector3{ 0.0f, -20.0f, 0.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Frost::Transform::Vector3{ 1000.0f, 500.0f, 1000.0f }
	);
	heightScale = 1.0f;
	filepath = "./resources/textures/heightmap.png";
	_scene.AddComponent<Frost::WorldTransform>(_terrain, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });

	auto material = Material();
	material.diffuseColor = DirectX::XMFLOAT3{ 1, 1, 1 };
	material.roughnessValue = 0;
	material.uvTiling = { 100,100 };
	material.filterMode = Material::FilterMode::LINEAR;
	material.diffuseTextures.push_back(TextureLibrary::Get().GetTexture("./resources/textures/dirt.jpg", TextureType::DIFFUSE));
	_scene.AddComponent<Frost::ModelRenderer>(_terrain, IsHeightMapRenderer(), filepath, material, TextureChannel::R, 50, heightScale);

	auto scale = _scene.GetComponent<Transform>(_terrain)->scale;
	auto texture = static_cast<TextureDX11*>(Frost::TextureLibrary::Get().GetTexture(filepath, TextureType::UNKNOWN).get());
	ShapeRefC heightFieldShape = Physics::Get().CreateHeightFieldShapeFromTexture(texture, TextureChannel::R, heightScale, scale);
	//ShapeRefC heightFieldShape = BoxShapeSettings(Vec3(1000.0f, 20.0f, 1000.0f)).Create().Get();
	BodyCreationSettings bodySettings(
		heightFieldShape,
		RVec3(0, -500, 0),
		Quat::sIdentity(),
		EMotionType::Static,
		ObjectLayers::NON_MOVING
	);

	_scene.AddComponent<Frost::RigidBody>(_terrain, bodySettings, _terrain, JPH::EActivation::Activate);
}