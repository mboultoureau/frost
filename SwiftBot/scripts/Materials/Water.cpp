#include "Materials/Water.h"

using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void Water::OnCreate()
    {
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .filePath = "./assets/Prefabs/Materials/Water/VS_Water.hlsl" };
        ShaderDesc hsDesc = { .type = ShaderType::Hull, .filePath = "./assets/Prefabs/Materials/Water/HS_Water.hlsl" };
        ShaderDesc dsDesc = { .type = ShaderType::Domain,
                              .filePath = "./assets/Prefabs/Materials/Water/DS_Water.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel, .filePath = "./assets/Prefabs/Materials/Water/PS_Water.hlsl" };

        auto vs = Shader::Create(vsDesc);
        auto hs = Shader::Create(hsDesc);
        auto ds = Shader::Create(dsDesc);
        auto ps = Shader::Create(psDesc);

        Material waveMat;
        waveMat.name = "TessellatedWaves";
        waveMat.customVertexShader = vs;
        waveMat.hullShader = hs;
        waveMat.domainShader = ds;
        waveMat.customPixelShader = ps;
        waveMat.backFaceCulling = false;

        _shaderParams.Time = 0.0f;
        _shaderParams.TopAmplitude = _waveAmplitude * 2;
        _shaderParams.TopFrequency = _waveFrequency;
        _shaderParams.TopWaveLength = _waveLength;
        _shaderParams.TessellationFactor = 32.0f;
        _shaderParams.BevelSize = 0.0001f;

        std::vector<uint8_t> paramData(sizeof(WaterMaterialParameters));
        memcpy(paramData.data(), &_shaderParams, sizeof(WaterMaterialParameters));
        waveMat.parameters = paramData;

        auto& staticMesh = GetGameObject().GetComponent<StaticMesh>();
        staticMesh.GetModel()->GetMaterials()[0] = std::move(waveMat);

        const auto& transform = GetGameObject().GetComponent<Transform>();

        _minWaterHeight = transform.position.y + transform.scale.y / 2 - _waveAmplitude / 2.0f;
        _maxWaterHeight = transform.position.y + transform.scale.y / 2 + _waveAmplitude / 2.0f;
    }

    void Water::OnUpdate(float deltaTime)
    {
        _shaderParams.Time += deltaTime;

        if (GetGameObject().HasComponent<StaticMesh>())
        {
            auto& mesh = GetGameObject().GetComponent<StaticMesh>();
            if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
            {
                std::vector<uint8_t> paramData(sizeof(WaterMaterialParameters));
                memcpy(paramData.data(), &_shaderParams, sizeof(WaterMaterialParameters));

                mesh.GetModel()->GetMaterials()[0].parameters = paramData;
            }
        }
    }

    void Water::OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime) {}
    void Water::OnCollisionStay(Frost::BodyOnContactParameters params, float deltaTime) {}
} // namespace GameLogic