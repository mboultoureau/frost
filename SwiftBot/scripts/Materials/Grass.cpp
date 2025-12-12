#include "Materials/Grass.h"
#include "Frost/Asset/ModelFactory.h"
#include "Frost/Renderer/Shader.h"

using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void Grass::OnCreate()
    {
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .filePath = "./assets/Prefabs/Materials/Grass/VS_Grass.hlsl" };
        ShaderDesc hsDesc = { .type = ShaderType::Hull, .filePath = "./assets/Prefabs/Materials/Grass/HS_Grass.hlsl" };
        ShaderDesc dsDesc = { .type = ShaderType::Domain,
                              .filePath = "./assets/Prefabs/Materials/Grass/DS_Grass.hlsl" };
        ShaderDesc gsDesc = { .type = ShaderType::Geometry,
                              .filePath = "./assets/Prefabs/Materials/Grass/GS_Grass.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel, .filePath = "./assets/Prefabs/Materials/Grass/PS_Grass.hlsl" };

        auto vs = Shader::Create(vsDesc);
        auto hs = Shader::Create(hsDesc);
        auto ds = Shader::Create(dsDesc);
        auto gs = Shader::Create(gsDesc);
        auto ps = Shader::Create(psDesc);

        Material grassMat;
        grassMat.name = "TessellatedGrass";
        grassMat.customVertexShader = vs;
        grassMat.hullShader = hs;
        grassMat.domainShader = ds;
        grassMat.geometryShader = gs;
        grassMat.customPixelShader = ps;

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

        if (GetGameObject().HasComponent<StaticMesh>())
        {
            auto& staticMesh = GetGameObject().GetComponent<StaticMesh>();
            if (staticMesh.GetModel())
            {
                staticMesh.GetModel()->GetMaterials()[0] = std::move(grassMat);
            }
        }
    }

    void Grass::OnUpdate(float deltaTime)
    {
        _params.Time += deltaTime;

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
} // namespace GameLogic