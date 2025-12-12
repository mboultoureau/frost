#include "Materials/Billboard.h"
#include "Frost/Asset/ModelFactory.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Asset/Texture.h"

using namespace Frost;
using namespace Frost::Component;

struct alignas(16) BillboardMaterialParameters
{
    Frost::Math::Vector4 CameraPosition;
    float BillboardSize;
    float padding1;
    float padding2;
    float padding3;
};

namespace GameLogic
{
    void Billboard::OnCreate()
    {
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .filePath = "./assets/Prefabs/Materials/Billboard/VS_Billboard.hlsl" };
        ShaderDesc gsDesc = { .type = ShaderType::Geometry,
                              .filePath = "./assets/Prefabs/Materials/Billboard/GS_Billboard.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .filePath = "./assets/Prefabs/Materials/Billboard/PS_Billboard.hlsl" };

        auto vs = Shader::Create(vsDesc);
        auto gs = Shader::Create(gsDesc);
        auto ps = Shader::Create(psDesc);

        Material billboardMat;
        billboardMat.name = "BillboardMaterial";
        billboardMat.customVertexShader = vs;
        billboardMat.geometryShader = gs;
        billboardMat.customPixelShader = ps;

        billboardMat.topology = Material::Topology::TRIANGLES;
        billboardMat.backFaceCulling = false;

        TextureConfig texConfig;
        texConfig.path = "./assets/Prefabs/Billboard/billboard.png";
        texConfig.format = Format::RGBA8_UNORM;
        auto texture = Texture::Create(texConfig);

        if (texture)
            billboardMat.albedoTextures.push_back(texture);

        _params.BillboardSize = 2.0f;

        std::vector<uint8_t> paramData(sizeof(BillboardMaterialParameters));
        memcpy(paramData.data(), &_params, sizeof(BillboardMaterialParameters));
        billboardMat.parameters = paramData;

        if (GetGameObject().HasComponent<StaticMesh>())
        {
            auto& mesh = GetGameObject().GetComponent<StaticMesh>();
            if (mesh.GetModel())
            {
                mesh.GetModel()->GetMaterials()[0] = std::move(billboardMat);
            }
        }
    }

    void Billboard::OnUpdate(float deltaTime)
    {
        if (GetGameObject().HasComponent<StaticMesh>())
        {
            auto& mesh = GetGameObject().GetComponent<StaticMesh>();
            if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
            {
                std::vector<uint8_t> paramData(sizeof(BillboardMaterialParameters));
                memcpy(paramData.data(), &_params, sizeof(BillboardMaterialParameters));

                mesh.GetModel()->GetMaterials()[0].parameters = paramData;
            }
        }
    }
} // namespace GameLogic