#include "Frost/Renderer/PostEffect/FogEffect.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"
#include "Frost/Renderer/DX11/CommandListDX11.h"

#include "Frost/Renderer/DX11/TextureDX11.h"

#undef max
#undef min
#include <algorithm>
#include <imgui.h>

namespace Frost
{
    struct alignas(16) FogConstants
    {
        float minDepth;
        float strength;
        float red;
        float green;
        float blue;
    };

    FogEffect::FogEffect()
    {
#ifdef FT_PLATFORM_WINDOWS
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .debugName = "VS_Fog",
                              .filePath = "../Frost/resources/shaders/PostEffect/Fog/VS_Fog.hlsl" };
        _vertexShader = std::make_unique<ShaderDX11>(vsDesc);

        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .debugName = "PS_Fog",
                              .filePath = "../Frost/resources/shaders/PostEffect/Fog/PS_Fog.hlsl" };
        _pixelShader = std::make_unique<ShaderDX11>(psDesc);
#endif

        auto* renderer = RendererAPI::GetRenderer();
        _constantsBuffer = renderer->CreateBuffer(
            BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(FogConstants), .dynamic = true });

        SamplerConfig samplerConfig = { .filter = Filter::MIN_MAG_MIP_LINEAR,
                                        .addressU = AddressMode::CLAMP,
                                        .addressV = AddressMode::CLAMP,
                                        .addressW = AddressMode::CLAMP };
        _sampler = std::make_unique<SamplerDX11>(samplerConfig);
    }

    void FogEffect::OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination)
    {
        FogConstants constants;
        constants.minDepth = _minDepth;
        constants.strength = _strength;
        constants.red = _red;
        constants.green = _green;
        constants.blue = _blue;

        _constantsBuffer->UpdateData(commandList, &constants, sizeof(constants));

        commandList->SetRenderTargets(1, &destination, nullptr);
        commandList->SetShader(_vertexShader.get());
        commandList->SetShader(_pixelShader.get());
        commandList->SetInputLayout(nullptr);

        commandList->SetTexture(source, 0);
        commandList->SetTexture(_depth, 1);
        commandList->SetSampler(_sampler.get(), 0);

        commandList->SetConstantBuffer(_constantsBuffer.get(), 0);

        commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        commandList->Draw(3, 0);
    }

    void FogEffect::OnImGuiRender(float deltaTime)
    {
        // Settings
        ImGui::Text("Settings");
        ImGui::Separator();
        ImGui::SliderFloat("Strength", &_strength, 0.0f, 2.0f, "%.05f");
        ImGui::SliderFloat("Red value", &_red, 0.0f, 1.0f, "%.4f");
        ImGui::SliderFloat("Green value", &_green, 0.0f, 1.0f, "%.4f");
        ImGui::SliderFloat("Blue value", &_blue, 0.0f, 1.0f, "%.4f");
    }
} // namespace Frost