#include "Waves.h"
#include "../Game.h"
#include "Frost/Asset/ModelFactory.h"
#include "Frost/Renderer/Shader.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Waves::Waves()
{
    Scene& scene = Game::GetScene();
    _planeObject = scene.CreateGameObject("WavesPlane");

    auto planeModel = ModelFactory::CreatePlane(10.0f, 10.0f);

    StaticMesh& meshComp = _planeObject.AddComponent<StaticMesh>(planeModel);
    Transform& transform = _planeObject.AddComponent<Transform>();
    transform.position = Vector3(0.0f, 0.0f, 0.0f);
	transform.Rotate(EulerAngles(0.0_deg, 0.0_deg, -180.0_deg));

    ShaderDesc vsDesc = { .type = ShaderType::Vertex, .filePath = "../Frost/resources/shaders/Material/Waves/VS_Waves.hlsl" };
    ShaderDesc hsDesc = { .type = ShaderType::Hull,   .filePath = "../Frost/resources/shaders/Material/Waves/HS_Waves.hlsl" };
    ShaderDesc dsDesc = { .type = ShaderType::Domain, .filePath = "../Frost/resources/shaders/Material/Waves/DS_Waves.hlsl" };
    ShaderDesc psDesc = { .type = ShaderType::Pixel,  .filePath = "../Frost/resources/shaders/Material/Waves/PS_Waves.hlsl" };

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

    _params.Time = 0.0f;
    _params.Amplitude = 0.5f;
    _params.Frequency = 1.0f;
    _params.TessellationFactor = 32.0f;

    std::vector<uint8_t> paramData(sizeof(WaveMaterialParameters));
    memcpy(paramData.data(), &_params, sizeof(WaveMaterialParameters));
    waveMat.parameters = paramData;

    planeModel->GetMaterials()[0] = std::move(waveMat);
}

void Waves::Update(float dt)
{
    _params.Time += dt;

    if (_planeObject.HasComponent<StaticMesh>())
    {
        auto& mesh = _planeObject.GetComponent<StaticMesh>();
        if (mesh.model && !mesh.model->GetMaterials().empty())
        {
            std::vector<uint8_t> paramData(sizeof(WaveMaterialParameters));
            memcpy(paramData.data(), &_params, sizeof(WaveMaterialParameters));

            mesh.model->GetMaterials()[0].parameters = paramData;
        }
    }
}