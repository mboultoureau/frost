#include "Billboard.h"
#include "../Game.h"
#include "../MainLayer.h"
#include "Frost/Asset/ModelFactory.h"
#include "Frost/Renderer/Shader.h"
#include "Player/PlayerCamera.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Renderer/Format.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Billboard::Billboard(Vector3 pos, float size, const std::string& texturePath)
{
    Scene& scene = Game::GetScene();
    _billboardObject = scene.CreateGameObject("BillboardObject");
    _billboardObject.AddScript<BillboardScript>(this);

    _params.BillboardSize = size;
    _params.CameraPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    TextureConfig config;
    config.path = texturePath;
    config.textureType = TextureType::BILLBOARD;
    config.format = Format::RGBA8_UNORM;
    config.channels = 4;

    _texture = Texture::Create(config);

    StaticMesh& meshComp = _billboardObject.AddComponent<StaticMesh>(MeshSourceCube{ 10.0f });

    Transform& transform = _billboardObject.AddComponent<Transform>();
    transform.position = pos;

    ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                          .filePath = "../Frost/resources/shaders/Material/Billboard/VS_Billboard.hlsl" };
    ShaderDesc gsDesc = { .type = ShaderType::Geometry,
                          .filePath = "../Frost/resources/shaders/Material/Billboard/GS_Billboard.hlsl" };
    ShaderDesc psDesc = { .type = ShaderType::Pixel,
                          .filePath = "../Frost/resources/shaders/Material/Billboard/PS_Billboard.hlsl" };

    Material billboardMat;
    billboardMat.name = "Billboard";

    billboardMat.customVertexShader = Shader::Create(vsDesc);
    billboardMat.geometryShader = Shader::Create(gsDesc);
    billboardMat.customPixelShader = Shader::Create(psDesc);

    billboardMat.topology = Material::Topology::TRIANGLES;
    billboardMat.backFaceCulling = false;

    billboardMat.albedoTextures.push_back(_texture);

    std::vector<uint8_t> paramData(sizeof(BillboardMaterialParameters));
    memcpy(paramData.data(), &_params, sizeof(BillboardMaterialParameters));
    billboardMat.parameters = paramData;

    meshComp.GetModel()->GetMaterials()[0] = std::move(billboardMat);
}

void
Billboard::_SetClosestPlayerPosToShader()
{
    auto mainLayer = Game::GetMainLayer();
    auto player = mainLayer->GetPlayer();

    if (player)
    {
        Scene& scene = Game::GetScene();

        const auto& playerCameraID = player->GetCamera()->GetCameraId();
        const auto* camTransform = scene.GetComponent<WorldTransform>(playerCameraID);

        if (camTransform)
        {
            _params.CameraPosition = DirectX::XMVectorSet(
                camTransform->position.x, camTransform->position.y, camTransform->position.z, 1.0f);
        }
    }
    else
    {
        _params.CameraPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

void
Billboard::UpdateShader(float dt)
{
    _SetClosestPlayerPosToShader();

    if (_billboardObject.HasComponent<StaticMesh>())
    {
        auto& mesh = _billboardObject.GetComponent<StaticMesh>();
        if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
        {
            Material& mat = mesh.GetModel()->GetMaterials()[0];

            std::vector<uint8_t> paramData(sizeof(BillboardMaterialParameters));
            memcpy(paramData.data(), &_params, sizeof(BillboardMaterialParameters));
            mat.parameters = paramData;
        }
    }
}

void
BillboardScript::OnUpdate(float deltaTime)
{
    _billboard->UpdateShader(deltaTime);
}

void
Billboard::DestroyObject()
{
    Game::GetScene().DestroyGameObject(_billboardObject);
}