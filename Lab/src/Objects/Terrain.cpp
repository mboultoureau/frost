#include "Terrain.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"

#include "Frost/Utils/Math/Vector.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace JPH;
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Terrain::Terrain()
{
	Scene& _scene = Game::GetScene();

	_terrain = _scene.CreateGameObject("Terrain");
	_scene.AddComponent<Transform>(
		_terrain,
		Vector3{ 0.0f, -20.0f, 0.0f },
		EulerAngles{ 0.0f, 0.0f, 0.0f },
		Vector3{ 1000.0f, 10.0f, 1000.0f }
	);
	heightScale = 1.0f;
	filepath = "./resources/textures/heightmap.png";
	_scene.AddComponent<WorldTransform>(_terrain, Vector3{ 0.0f, 0.0f, 0.0f });

	auto transform = _scene.GetComponent<Transform>(_terrain);

	Material material;
	material.albedo = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
	material.roughness = 0;
	material.uvTiling = { 100, 100 };
	material.filter = Material::FilterMode::LINEAR;

	TextureConfig heightmapConfig;
	heightmapConfig.textureType = TextureType::HEIGHTMAP;
	heightmapConfig.path = filepath;

	HeightMapConfig heightMapConfig {
		material,
		AssetManager::LoadAsset(filepath, heightmapConfig),
		50,
		transform->scale
	};

	//material.albedoTextures.push_back(AssetManager::LoadAsset<Texture>("./resources/textures/dirt.jpg", "./resources/textures/dirt.jpg", TextureType::DIFFUSE));
	std::shared_ptr<Model> terrainMesh = ModelFactory::CreateFromHeightMap(heightMapConfig);
	_scene.AddComponent<StaticMesh>(_terrain, terrainMesh);


	//auto texture = static_cast<TextureDX11*>(Frost::TextureLibrary::Get().GetTexture(filepath, TextureType::UNKNOWN).get());
	//ShapeRefC heightFieldShape = Physics::Get().CreateHeightFieldShapeFromTexture(texture, TextureChannel::R, heightScale, scale);
	//ShapeRefC heightFieldShape = BoxShapeSettings(Vec3(1000.0f, 20.0f, 1000.0f)).Create().Get();
	ShapeRefC heightFieldShape = ShapeFactory::CreateHeightMap(heightMapConfig);

	BodyCreationSettings bodySettings(
		heightFieldShape,
		RVec3(0, -500, 0),
		Quat::sIdentity(),
		EMotionType::Static,
		ObjectLayers::NON_MOVING
	);

	_scene.AddComponent<RigidBody>(_terrain, bodySettings, _terrain, JPH::EActivation::Activate);

}