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
    using namespace JPH;
    Scene& _scene = Game::GetScene();

    const std::string filepathHeightmap = "./resources/textures/image.png";

    MeshSourceHeightMap terrainConfig;
    terrainConfig.texturePath = filepathHeightmap;
    terrainConfig.width = 500.0f;
    terrainConfig.depth = 500.0f;
    terrainConfig.minHeight = 0.0f;
    terrainConfig.maxHeight = 125.0f;
    terrainConfig.segmentsWidth = 512;
    terrainConfig.segmentsDepth = 512;

    TextureConfig heightmapTexConfig;
    heightmapTexConfig.textureType = TextureType::HEIGHTMAP;
    heightmapTexConfig.path = filepathHeightmap;

    TextureConfig rainbowConfig;
    rainbowConfig.textureType = TextureType::DIFFUSE;
    rainbowConfig.path = "./resources/textures/rainbow.jpg";

    std::shared_ptr<Texture> heightmapTexture = Texture::Create(heightmapTexConfig);
    std::shared_ptr<Texture> rainbowTexture = Texture::Create(rainbowConfig);

    _terrain = _scene.CreateGameObject("Terrain");

    auto& transform = _terrain.AddComponent<Transform>(
        Vector3{ -180.0f, 0.0f, 0.0f }, EulerAngles{ 0.0f, 0.0f, 0.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
    _terrain.AddComponent<WorldTransform>(Vector3{ 0.0f, 0.0f, 0.0f });

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

        _terrain.AddComponent<RigidBody>(bodySettings, _terrain, JPH::EActivation::Activate);
    }
    else
    {
        FT_ENGINE_ERROR("Terrain RigidBody creation failed due to null shape.");
    }

    // Ground
    auto ground = _scene.CreateGameObject("Ground");
    ground.AddComponent<Transform>(
        Vector3{ 0.0f, -10.0f, -150.0f }, EulerAngles{ 0.0f, 0.0f, 0.0f }, Vector3{ 1000.0f, 100.0f, 1000.0f });
    ground.AddComponent<WorldTransform>(Vector3{ 0.0f, 0.0f, 0.0f });

    /*
    auto material2 = Material();
    material2.diffuseColor = DirectX::XMFLOAT3{ 0.5, 0.5, 1 };
    material2.roughnessValue = 0;
    material2.emissiveColor = DirectX::XMFLOAT3{ 0.1, 0.1, 0.5 };
    material2.uvTiling = { 10,10 };
    material2.filterMode = Material::FilterMode::LINEAR;
    material2.diffuseTextures.push_back(TextureLibrary::Get().GetTexture("./resources/textures/dirt.jpg",
    TextureType::DIFFUSE)); _scene.AddComponent<Frost::ModelRenderer>(ground,
    IsHeightMapRenderer(), filepath, material2, TextureChannel::G, 50,
    heightScale);
    */
}
