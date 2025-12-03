#include "ToonEffect.h"
#include "Frost/Renderer/RendererAPI.h"
#include <imgui.h>
#include <Frost/Renderer/DX11/ShaderDX11.h>
#include <Frost/Renderer/DX11/SamplerDX11.h>
#include <Frost/Core/Application.h>

namespace Frost
{
    ToonEffect::ToonEffect() : constants{}
    {
#ifdef FT_PLATFORM_WINDOWS
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .debugName = "VS_Toon",
                              .filePath = "../Frost/resources/shaders/PostEffect/ToonShading/VS_Toon.hlsl" };
        _vertexShader = std::make_unique<ShaderDX11>(vsDesc);

        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .debugName = "PS_Toon",
                              .filePath = "../Frost/resources/shaders/PostEffect/ToonShading/PS_Toon.hlsl" };
        _pixelShader = std::make_unique<ShaderDX11>(psDesc);
#endif

        auto* renderer = RendererAPI::GetRenderer();
        _constantsBuffer = renderer->CreateBuffer(
            BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(ToonConstants), .dynamic = true });

        SamplerConfig samplerConfig = { .filter = Filter::MIN_MAG_MIP_LINEAR,
                                        .addressU = AddressMode::CLAMP,
                                        .addressV = AddressMode::CLAMP,
                                        .addressW = AddressMode::CLAMP };
        _sampler = std::make_unique<SamplerDX11>(samplerConfig);
    }

    void DrawColorSteps(const char* label, int& count, DirectX::XMFLOAT4* steps)
    {
        ImGui::SliderInt((std::string(label) + " Step Count").c_str(), &count, 1, MAX_STEPS);
        for (int i = 0; i < count; i++)
        {
            ImGui::SliderFloat(
                (std::string(label) + " Step " + std::to_string(i)).c_str(), &steps[i].x, 0.f, 1.f, "%.4f");
        }
    }

    void ToonEffect::OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination)
    {
        constants.redStepCount = 5;
        constants.greenStepCount = 5;
        constants.blueStepCount = 5;
        int pad0;

        // --- RED ---
        constants.redSteps[0] = DirectX::XMFLOAT4(0.2f, 0.f, 0.f, 0.f);
        constants.redSteps[1] = DirectX::XMFLOAT4(0.4f, 0.f, 0.f, 0.f);
        constants.redSteps[2] = DirectX::XMFLOAT4(0.6f, 0.f, 0.f, 0.f);
        constants.redSteps[3] = DirectX::XMFLOAT4(0.8f, 0.f, 0.f, 0.f);
        constants.redSteps[3] = DirectX::XMFLOAT4(1.0f, 0.f, 0.f, 0.f);

        // --- GREEN ---
        constants.greenSteps[0] = DirectX::XMFLOAT4(0.2f, 0.f, 0.f, 0.f);
        constants.greenSteps[1] = DirectX::XMFLOAT4(0.4f, 0.f, 0.f, 0.f);
        constants.greenSteps[2] = DirectX::XMFLOAT4(0.6f, 0.f, 0.f, 0.f);
        constants.greenSteps[3] = DirectX::XMFLOAT4(0.8f, 0.f, 0.f, 0.f);
        constants.greenSteps[3] = DirectX::XMFLOAT4(1.0f, 0.f, 0.f, 0.f);

        // --- BLUE ---
        constants.blueSteps[0] = DirectX::XMFLOAT4(0.2f, 0.f, 0.f, 0.f);
        constants.blueSteps[1] = DirectX::XMFLOAT4(0.4f, 0.f, 0.f, 0.f);
        constants.blueSteps[2] = DirectX::XMFLOAT4(0.6f, 0.f, 0.f, 0.f);
        constants.blueSteps[3] = DirectX::XMFLOAT4(0.8f, 0.f, 0.f, 0.f);
        constants.blueSteps[3] = DirectX::XMFLOAT4(1.0f, 0.f, 0.f, 0.f);

        constants.edgeThreshold = 0.3f;     // Seuil normales (ex: 0.3 a 0.6)
        constants.edgeStrength = 1.0f;      // Intensite du contour (ex: 1.0)
        constants.depthThreshold = 0.01f;   // Seuil profondeur (ex: 0.01 a 0.1)
        constants.depthSensitivity = 50.0f; // Sensibilite profondeur (ex: 50.0)

        constants.normalStrength = 1.0f; // Multiplicateur pour l'assombrissement (ex: 1.0)
        constants.normalMin = 0.0f;      // Valeur minimale de normal (ex: 0.0)
        constants.normalMax = 1.0f;      // Valeur maximale de normal (ex: 1.0)

        constants.edgeColor = { 0.0f, 0.0f, 0.0f };

        constants.texelSize = { 1.0f / Application::GetWindow()->GetWidth(),
                                1.0f / Application::GetWindow()->GetHeight() };

        FT_ASSERT(constants.redStepCount != 0, "red step count must be strictly greater than 0");
        FT_ASSERT(constants.greenStepCount != 0, "green step count must be strictly greater than 0");
        FT_ASSERT(constants.blueStepCount != 0, "blue step count must be strictly greater than 0");

        _constantsBuffer->UpdateData(commandList, &constants, sizeof(constants));

        commandList->SetRenderTargets(1, &destination, nullptr);
        commandList->SetShader(_vertexShader.get());
        commandList->SetShader(_pixelShader.get());
        commandList->SetInputLayout(nullptr);

        commandList->SetTexture(source, 0);
        commandList->SetTexture(_normal, 1);
        commandList->SetSampler(_sampler.get(), 0);

        commandList->SetConstantBuffer(_constantsBuffer.get(), 0);

        commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        commandList->Draw(3, 0);
    }

    void ToonEffect::OnImGuiRender(float deltaTime)
    {
        // Settings
        ImGui::Text("Settings");
        ImGui::Separator();

        DrawColorSteps("Red", constants.redStepCount, constants.redSteps);
        DrawColorSteps("Green", constants.greenStepCount, constants.greenSteps);
        DrawColorSteps("Blue", constants.blueStepCount, constants.blueSteps);
    }
} // namespace Frost