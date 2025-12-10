#include "Grass.h"
#include "Frost/Asset/ModelFactory.h"
#include "Frost/Renderer/Shader.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void GrassScript::OnCreate()
    {
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
        _params.BladeWidth = 0.25f;
        _params.BladeWidthRandom = 0.02f;
        _params.BladeHeight = 1.5f;
        _params.BladeHeightRandom = 0.3f;
        _params.BladeForward = 0.38f;
        _params.BladeCurve = 2.0f;
        _params.BendRotationRandom = 0.2f;
        _params.TessellationFactor = 32.0f;
        _params.WindStrength = 1.0f;

        std::vector<uint8_t> paramData(sizeof(GrassMaterialParameters));
        memcpy(paramData.data(), &_params, sizeof(GrassMaterialParameters));
        grassMat.parameters = paramData;

        auto& meshComp = GetGameObject().GetComponent<StaticMesh>();
        meshComp.GetModel()->GetMaterials()[0] = std::move(grassMat);
    }

    void GrassScript::_SetClosestPlayerPosToShader()
    {

        /*
        auto mainLayer = GetMainLayer();
        if (mainLayer)
        {
            auto player = mainLayer->GetPlayer();
            if (player)
            {
                auto& scene = Game::GetScene();
                auto playerCameraId = player->GetCamera()->GetCameraId();
                // On suppose que GetComponent<WorldTransform> retourne un pointeur ou un type déréférençable
                auto camPos = scene.GetComponent<WorldTransform>(playerCameraId);

                if (camPos)
                {
                    _params.CameraPosition = { camPos->position.x, camPos->position.y, camPos->position.z };
                    return;
                }
            }
        }
        */

        _params.CameraPosition = { 0.0f, 0.0f, 0.0f };
    }

    void GrassScript::_UpdateMaterialParameters()
    {
        if (GetGameObject().HasComponent<StaticMesh>())
        {
            auto& mesh = GetGameObject().GetComponent<StaticMesh>();
            if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
            {
                std::vector<uint8_t> paramData(sizeof(GrassMaterialParameters));
                memcpy(paramData.data(), &_params, sizeof(GrassMaterialParameters));

                mesh.GetModel()->GetMaterials()[0].parameters = paramData;
            }
        }
    }

    void GrassScript::OnUpdate(float deltaTime)
    {
        _params.Time += deltaTime;
        _SetClosestPlayerPosToShader();
        _UpdateMaterialParameters();
    }
} // namespace GameLogic