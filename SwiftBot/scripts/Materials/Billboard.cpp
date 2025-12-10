#include "Billboard.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Core/Game.h"

using namespace Frost;
using namespace Frost::Math;
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
    void BillboardScript::OnCreate()
    {
        if (!GetGameObject().HasComponent<StaticMesh>())
        {
            FT_ASSERT(false, "Billboard GameObject missing StaticMesh component!");
            return;
        }

        auto& meshComp = GetGameObject().GetComponent<StaticMesh>();
        auto model = meshComp.GetModel();

        if (model && !model->GetMaterials().empty())
        {
            ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                                  .filePath = "../Frost/resources/shaders/Material/Billboard/VS_Billboard.hlsl" };
            ShaderDesc gsDesc = { .type = ShaderType::Geometry,
                                  .filePath = "../Frost/resources/shaders/Material/Billboard/GS_Billboard.hlsl" };
            ShaderDesc psDesc = { .type = ShaderType::Pixel,
                                  .filePath = "../Frost/resources/shaders/Material/Billboard/PS_Billboard.hlsl" };

            auto vs = Shader::Create(vsDesc);
            auto gs = Shader::Create(gsDesc);
            auto ps = Shader::Create(psDesc);

            TextureConfig billboardConfig;
            billboardConfig.path = "../Frost/resources/textures/my_billboard_image.png";
            billboardConfig.textureType = TextureType::BILLBOARD;
            auto billboardTexture = Texture::Create(billboardConfig);

            Material billboardMat = model->GetMaterials()[0];
            billboardMat.name = "RealBillboard";
            billboardMat.customVertexShader = vs;
            billboardMat.geometryShader = gs;
            billboardMat.customPixelShader = ps;

            billboardMat.topology = Material::Topology::TRIANGLES;

            billboardMat.albedoTextures.clear();
            if (billboardTexture)
            {
                billboardMat.albedoTextures.push_back(std::move(billboardTexture));
            }

            billboardMat.backFaceCulling = false;

            BillboardMaterialParameters params = {};
            params.BillboardSize = 5.0f;

            std::vector<uint8_t> paramData(sizeof(BillboardMaterialParameters));
            memcpy(paramData.data(), &params, sizeof(BillboardMaterialParameters));
            billboardMat.parameters = paramData;

            model->GetMaterials()[0] = std::move(billboardMat);
        }
    }

    void BillboardScript::OnUpdate(float deltaTime)
    {
        if (GetGameObject().HasComponent<StaticMesh>())
        {
            Frost::Math::Vector3 camPos = { 0.0f, 0.0f, 0.0f };

            auto& mesh = GetGameObject().GetComponent<StaticMesh>();
            if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
            {
                auto& mat = mesh.GetModel()->GetMaterials()[0];
                if (mat.parameters.size() >= sizeof(BillboardMaterialParameters))
                {
                    BillboardMaterialParameters params;
                    memcpy(&params, mat.parameters.data(), sizeof(BillboardMaterialParameters));

                    params.CameraPosition = { camPos.x, camPos.y, camPos.z, 1.0f };

                    memcpy(mat.parameters.data(), &params, sizeof(BillboardMaterialParameters));
                }
            }
        }
    }
} // namespace GameLogic