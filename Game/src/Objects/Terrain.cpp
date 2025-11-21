#include "Terrain.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Terrain::Terrain()
{
	using namespace JPH;
	Scene& _scene = Game::GetScene();


	// Road
	_terrain = _scene.CreateGameObject("Terrain");
	_terrain.AddComponent<Transform>(
		Vector3{ 0.0f, -10.0f, -150.0f },
		EulerAngles{ 0.0f, 0.0f, 0.0f },
		Vector3{ 1000.0f, 10.0f, 1000.0f }
	);
	auto& transform = _terrain.GetComponent<Transform>();

	const std::string filepathHeightmap{ "./resources/textures/Map_RainbowRoad.png" };
	_terrain.AddComponent<WorldTransform>(Vector3{ 0.0f, 0.0f, 0.0f });

	TextureConfig heightmapConfig;
	heightmapConfig.textureType = TextureType::HEIGHTMAP;
	heightmapConfig.path = filepathHeightmap;

	TextureConfig rainbowConfig;
	rainbowConfig.textureType = TextureType::DIFFUSE;
	rainbowConfig.path = "./resources/textures/rainbow.jpg";

	std::shared_ptr<Texture> heightmapTexture = AssetManager::LoadAsset(filepathHeightmap, heightmapConfig);
	std::shared_ptr<Texture> rainbowTexture = AssetManager::LoadAsset(rainbowConfig.path, rainbowConfig);

	Material material;
	material.albedoTextures.push_back(rainbowTexture);
	material.albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	material.roughness = 0;
	material.uvTiling = { 100, 100 };
	material.filter = Material::FilterMode::LINEAR;

	HeightMapConfig heightMapConfig {
		material,
		heightmapTexture,
		50,
		transform.scale
	};

	std::shared_ptr<Model> terrainMesh = ModelFactory::CreateFromHeightMap(heightMapConfig);
	_terrain.AddComponent<StaticMesh>(terrainMesh);

	/*
	auto material = Material();
	material.diffuseColor = DirectX::XMFLOAT3{ 1, 1, 1 };
	material.roughnessValue = 0;
//	material.emissiveColor = DirectX::XMFLOAT3{ 0.5, 0.5, 0.5 };
	material.uvTiling = { 100,100 };
	material.filterMode = Material::FilterMode::LINEAR;
	material.diffuseTextures.push_back(TextureLibrary::Get().GetTexture("./resources/textures/rainbow.jpg", TextureType::DIFFUSE));
	_scene.AddComponent<Frost::ModelRenderer>(_terrain, IsHeightMapRenderer(), filepath, material, TextureChannel::R, 50, heightScale);
	*/
	ShapeRefC heightFieldShape = ShapeFactory::CreateHeightMap(heightMapConfig);
	//ShapeRefC heightFieldShape = BoxShapeSettings(Vec3(1000.0f, 20.0f, 1000.0f)).Create().Get();
	BodyCreationSettings bodySettings(
		heightFieldShape,
		RVec3(0, -10, -150),
		Quat::sIdentity(),
		EMotionType::Static,
		ObjectLayers::NON_MOVING
	);
	bodySettings.mRestitution = 0.0f;

	_terrain.AddComponent<RigidBody>(bodySettings, _terrain, JPH::EActivation::Activate);
	auto bodyId = _terrain.GetComponent<RigidBody>().physicBody->bodyId;


	// Ground 
	auto ground = _scene.CreateGameObject("Ground");
	ground.AddComponent<Transform>(
		Vector3{ 0.0f, -10.0f, -150.0f },
		EulerAngles{ 0.0f, 0.0f, 0.0f },
		Vector3{ 1000.0f, 100.0f, 1000.0f }
	);
	ground.AddComponent<WorldTransform>(Vector3{ 0.0f, 0.0f, 0.0f });

	/*
	auto material2 = Material();
	material2.diffuseColor = DirectX::XMFLOAT3{ 0.5, 0.5, 1 };
	material2.roughnessValue = 0;
	material2.emissiveColor = DirectX::XMFLOAT3{ 0.1, 0.1, 0.5 };
	material2.uvTiling = { 10,10 };
	material2.filterMode = Material::FilterMode::LINEAR;
	material2.diffuseTextures.push_back(TextureLibrary::Get().GetTexture("./resources/textures/dirt.jpg", TextureType::DIFFUSE));
	_scene.AddComponent<Frost::ModelRenderer>(ground, IsHeightMapRenderer(), filepath, material2, TextureChannel::G, 50, heightScale);
	*/
}
