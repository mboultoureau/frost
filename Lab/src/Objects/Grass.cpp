#include "Grass.h"
#include "../Game.h"
#include "../MainLayer.h"
#include "Frost/Asset/ModelFactory.h"
#include "Frost/Renderer/Shader.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Grass::Grass()
{
    Scene& scene = Game::GetScene();
    _planeObject = scene.CreateGameObject("GrassPlane");

    StaticMesh& meshComp = _planeObject.AddComponent<StaticMesh>(MeshSourcePlane{ 10.0f, 10.0f });
    Transform& transform = _planeObject.AddComponent<Transform>();
    transform.position = Vector3(10.0f, 0.0f, 0.0f);
    transform.Rotate(EulerAngles(180.0_deg, 0.0_deg, 0.0_deg));

    ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                          .filePath = "../Frost/resources/shaders/Material/Grass/VS_Grass.hlsl" };
    ShaderDesc gsDesc = { .type = ShaderType::Geometry,
                          .filePath = "../Frost/resources/shaders/Material/Grass/GS_Grass.hlsl" };
    ShaderDesc dsDesc = { .type = ShaderType::Domain,
                          .filePath = "../Frost/resources/shaders/Material/Grass/DS_Grass.hlsl" };
    ShaderDesc hsDesc = { .type = ShaderType::Hull,
                          .filePath = "../Frost/resources/shaders/Material/Grass/HS_Grass.hlsl" };
    ShaderDesc psDesc = { .type = ShaderType::Pixel,
                          .filePath = "../Frost/resources/shaders/Material/Grass/PS_Grass.hlsl" };

    auto vs = Shader::Create(vsDesc);
    auto gs = Shader::Create(gsDesc);
    auto ds = Shader::Create(dsDesc);
    auto ps = Shader::Create(psDesc);
    auto hs = Shader::Create(hsDesc);

    Material grassMat;
    grassMat.name = "TessellatedGrass";
    grassMat.customVertexShader = vs;
    grassMat.geometryShader = gs;
    grassMat.domainShader = ds;
    grassMat.customPixelShader = ps;
    grassMat.hullShader = hs;
    grassMat.topology = Material::Topology::PATCH_3;
    grassMat.backFaceCulling = false;

    _params.Time = 0.0f;
    _params.TopColor = DirectX::XMFLOAT4(0.4f, 0.8f, 0.3f, 1.0f);
    _params.BottomColor = DirectX::XMFLOAT4(0.2f, 0.4f, 0.1f, 1.0f);
    _params.BladeWidth = 0.05f;
    _params.BladeWidthRandom = 0.02f;
    _params.BladeHeight = 0.5f;
    _params.BladeHeightRandom = 0.3f;
    _params.BladeForward = 0.38f;
    _params.BladeCurve = 2.0f;
    _params.BendRotationRandom = 0.2f;
    _params.TessellationFactor = 64.0f;
    _params.WindStrength = 1.0f;
    //_params.WindFrequency = DirectX::XMFLOAT2(0.05f, 0.05f);

    std::vector<uint8_t> paramData(sizeof(GrassMaterialParameters));
    memcpy(paramData.data(), &_params, sizeof(GrassMaterialParameters));
    grassMat.parameters = paramData;

    meshComp.GetModel()->GetMaterials()[0] = std::move(grassMat);
}

void
Grass::_SetClosestPlayerPosToShader()
{
    auto player = Game::GetMainLayer()->GetPlayer();
    if (player)
    {
        auto camPos = player->GetPlayerID().GetComponent<WorldTransform>().position;
        _params.CameraPosition = { camPos.x, camPos.y, camPos.z };
    }
    else
    {
        _params.CameraPosition = { 0.0f, 0.0f, 0.0f };
    }
}

void
Grass::Update(float dt)
{
    _params.Time += dt;
    _SetClosestPlayerPosToShader();

    if (_planeObject.HasComponent<StaticMesh>())
    {
        auto& mesh = _planeObject.GetComponent<StaticMesh>();
        if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
        {
            std::vector<uint8_t> paramData(sizeof(GrassMaterialParameters));
            memcpy(paramData.data(), &_params, sizeof(GrassMaterialParameters));

            mesh.GetModel()->GetMaterials()[0].parameters = paramData;
        }
    }
}