#include "Terrain.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Terrain::Terrain()
{

    const std::string filepathHeightmapBot = "./resources/textures/MapDesespoir-min.png";

    const std::string textureGrass = "./resources/textures/grass.png";
    const std::string textureDirt = "./resources/textures/dirt.jpg";
    const std::string textureMetal = "./resources/textures/metal.png";

    MakeTerrain(Frost::Math::Vector3(), filepathHeightmapBot, textureDirt);
    MakeCube(
        Vector3{ -10.0f, 55.0f, 50 }, EulerAngles{ -45.0_deg, 0, 15_deg }, Vector3{ 1, 15, 50 }, textureMetal, true);

    MakeCube(Vector3{ 9.0f, 70, -47 }, EulerAngles{ 0_deg, 0, 0_deg }, Vector3{ 15, 1, 50 }, textureMetal, true);
}

void
Terrain::MakeTerrain(Frost::Math::Vector3 pos, std::string filepathHeightmap, std::string texturePath)
{
    using namespace JPH;
    Scene& _scene = Game::GetScene();

    MeshSourceHeightMap terrainConfig;
    terrainConfig.texturePath = filepathHeightmap;
    terrainConfig.width = 500.f;
    terrainConfig.depth = 500.f;
    terrainConfig.minHeight = 0.0f;
    terrainConfig.maxHeight = 125.0f;
    terrainConfig.segmentsWidth = 512;
    terrainConfig.segmentsDepth = 512;

    TextureConfig heightmapTexConfig;
    heightmapTexConfig.textureType = TextureType::HEIGHTMAP;
    heightmapTexConfig.path = filepathHeightmap;

    TextureConfig rainbowConfig;
    rainbowConfig.textureType = TextureType::DIFFUSE;
    rainbowConfig.path = texturePath;

    std::shared_ptr<Texture> heightmapTexture = Texture::Create(heightmapTexConfig);
    std::shared_ptr<Texture> rainbowTexture = Texture::Create(rainbowConfig);

    _terrain = _scene.CreateGameObject("Terrain");

    auto& transform = _terrain.AddComponent<Transform>(
        pos, Frost::Math::EulerAngles{ 0.0f, 0.0f, 0.0f }, Frost::Math::Vector3{ 1.0f, 1.0f, 1.0f });
    _terrain.AddComponent<WorldTransform>(Frost::Math::Vector3{ 0.0f, 0.0f, 0.0f });

    Material material;
    material.albedoTextures.push_back(rainbowTexture);
    material.albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    material.uvTiling = { 100, 100 };

    auto& staticMesh = _terrain.AddComponent<StaticMesh>(terrainConfig);
    staticMesh.GetModel()->GetMaterials()[0] = std::move(material);

    ShapeRefC heightFieldShape = ShapeFactory::CreateHeightField(terrainConfig, heightmapTexture);

    if (heightFieldShape)
    {
        BodyCreationSettings bodySettings(heightFieldShape,
                                          vector_cast<JPH::RVec3>(transform.position),
                                          vector_cast<JPH::Quat>(transform.rotation),
                                          EMotionType::Static,
                                          ObjectLayers::NON_MOVING);
        bodySettings.mRestitution = 0.0f;

        auto bodyId = Physics::CreateAndAddBody(bodySettings, _terrain.GetHandle(), JPH::EActivation::DontActivate);

        _terrain.AddComponent<RigidBody>(bodyId);
    }
    else
    {
        FT_ENGINE_ERROR("Terrain RigidBody creation failed due to null shape.");
    }
}

void
Terrain::MakeCube(Vector3 pos, EulerAngles rot, Vector3 scale, std::string texturePath, bool isPhysics)
{
    using namespace JPH;

    Scene& scene = Game::GetScene();

    auto boost = scene.CreateGameObject("cube");
    auto cubeModel = MeshSourceCube{ 1.0f };
    boost.AddComponent<Transform>(pos, rot, scale);

    Material waveMat;
    waveMat.name = "cube";

    TextureConfig texture;
    texture.path = texturePath;
    waveMat.albedoTextures.push_back(std::make_shared<TextureDX11>(texture));
    waveMat.backFaceCulling = false;
    auto mesh = boost.AddComponent<StaticMesh>(cubeModel);
    mesh.GetModel()->GetMaterials()[0] = std::move(waveMat);

    // Create water sensor. We use this to detect which bodies entered the water
    // (in this sample we could have assumed everything is in the water)
    if (isPhysics)
    {
        auto a = ShapeBox{};
        a.halfExtent = Vector3{ scale.x / 2, scale.y / 2, scale.z / 2 };
        auto& rb = boost.AddComponent<RigidBody>(a, ObjectLayers::NON_MOVING);
    }
}