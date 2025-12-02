#include "SphereCustomShader.h"
#include "../Game.h"

SphereCustomShader::SphereCustomShader()
{
    Scene& _scene = Game::GetScene();
    _sphere = _scene.CreateGameObject("SphereCustomShader");
    StaticMesh& mesh = _sphere.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/plane.fbx" });
    Transform& transform = _sphere.AddComponent<Transform>();

    transform.position = Math::Vector3(0.0f, -1.0f, 0.0f);
    transform.scale = Math::Vector3(50.0f, 50.0f, 1.0f);

    // Custom Shader Material
    struct alignas(16) GridMaterialParameters
    {
        Math::Color4 GridColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        Math::Color4 BaseColor = { 0.5f, 0.5f, 0.5f, 1.0f };
        float GridScale = 1.0f;
        float LineThickness = 0.02f;
        float Padding[2];
    };

    Frost::ShaderDesc vsDesc = { .type = Frost::ShaderType::Vertex,
                                 .filePath = "../Frost/resources/shaders/Material/Grid/VS_Grid.hlsl" };
    Frost::ShaderDesc psDesc = { .type = Frost::ShaderType::Pixel,
                                 .filePath = "../Frost/resources/shaders/Material/Grid/PS_Grid.hlsl" };

    auto gridVS = Shader::Create(vsDesc);
    auto gridPS = Shader::Create(psDesc);

    GridMaterialParameters params;
    params.GridColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    params.BaseColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    params.GridScale = 2.0f;
    params.LineThickness = 0.1f;

    std::vector<uint8_t> paramData(sizeof(GridMaterialParameters));
    memcpy(paramData.data(), &params, sizeof(GridMaterialParameters));

    Frost::Material gridMat;
    gridMat.name = "InfiniteGrid";
    gridMat.customVertexShader = gridVS;
    gridMat.customPixelShader = gridPS;
    gridMat.parameters = paramData;

    //    auto& materials = mesh.GetModel()->GetMaterials();
    //    materials[0] = gridMat;
}
