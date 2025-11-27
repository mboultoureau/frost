#include "Frost/Renderer/PostEffect/RadialBlurEffect.h"
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
    struct alignas(16) RadialBlurConstants
    {
        Math::Vector2 center;
        float strength;
        int sampleCount;
	};

    RadialBlurEffect::RadialBlurEffect()
    {
#ifdef FT_PLATFORM_WINDOWS
        ShaderDesc vsDesc = { .type = ShaderType::Vertex, .debugName = "VS_RadialBlur", .filePath = "../Frost/resources/shaders/PostEffect/VS_RadialBlur.hlsl" };
        _vertexShader = std::make_unique<ShaderDX11>(vsDesc);

        ShaderDesc psDesc = { .type = ShaderType::Pixel, .debugName = "PS_RadialBlur", .filePath = "../Frost/resources/shaders/PostEffect/PS_RadialBlur.hlsl" };
        _pixelShader = std::make_unique<ShaderDX11>(psDesc);
#endif

        auto* renderer = RendererAPI::GetRenderer();
        _constantsBuffer = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(RadialBlurConstants), .dynamic = true, .debugName = "CB_RadialBlur" });

        SamplerConfig samplerConfig = { .filter = Filter::MIN_MAG_MIP_LINEAR, .addressU = AddressMode::CLAMP, .addressV = AddressMode::CLAMP, .addressW = AddressMode::CLAMP };
        _sampler = std::make_unique<SamplerDX11>(samplerConfig);
    }

    void RadialBlurEffect::OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination)
    {
        RadialBlurConstants constants;
        constants.center = _center;
        constants.strength = _strength;
        constants.sampleCount = _sampleCount;
        _constantsBuffer->UpdateData(commandList, &constants, sizeof(constants));

        commandList->SetRenderTargets(1, &destination, nullptr);
        commandList->SetShader(_vertexShader.get());
        commandList->SetShader(_pixelShader.get());
        commandList->SetInputLayout(nullptr);

        commandList->SetTexture(source, 0);
        commandList->SetSampler(_sampler.get(), 0);
        commandList->SetConstantBuffer(_constantsBuffer.get(), 0);

        commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        commandList->Draw(3, 0);
    }

    void RadialBlurEffect::OnImGuiRender(float deltaTime)
    {
        // Settings
        ImGui::Text("Settings");
        ImGui::Separator();
        ImGui::SliderFloat("Strength", &_strength, 0.0f, 0.2f, "%.4f");
        ImGui::SliderInt("Sample Count", &_sampleCount, 1, 30);
        ImGui::DragFloat2("Center", &_center.x, 0.005f, 0.0f, 1.0f, "%.3f");

        // Center visualization
        ImGui::Text("Center Control");
        ImGui::Separator();
        ImGui::Text("Click or drag in the box below to set the blur center:");

        float availableWidth = ImGui::GetContentRegionAvail().x;
        ImVec2 zoneSize(availableWidth, availableWidth * (9.0f / 16.0f));

        ImVec2 zoneTopLeft = ImGui::GetCursorScreenPos();
        ImVec2 zoneBottomRight = ImVec2(zoneTopLeft.x + zoneSize.x, zoneTopLeft.y + zoneSize.y);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(zoneTopLeft, zoneBottomRight, IM_COL32(40, 40, 45, 255));
        drawList->AddRect(zoneTopLeft, zoneBottomRight, IM_COL32(150, 150, 150, 255));
        ImGui::InvisibleButton("##center_zone", zoneSize);

        if (ImGui::IsItemActive())
        {
            ImVec2 mousePos = ImGui::GetMousePos();

            float relativeX = mousePos.x - zoneTopLeft.x;
            float relativeY = mousePos.y - zoneTopLeft.y;

            float normalizedX = relativeX / zoneSize.x;
            float normalizedY = relativeY / zoneSize.y;

            _center.x = std::max(0.0f, std::min(1.0f, normalizedX));
            _center.y = std::max(0.0f, std::min(1.0f, normalizedY));
        }

        ImVec2 pointPos(zoneTopLeft.x + _center.x * zoneSize.x, zoneTopLeft.y + _center.y * zoneSize.y);
        drawList->AddCircleFilled(pointPos, 6.0f, IM_COL32(255, 50, 50, 255));
        drawList->AddCircle(pointPos, 7.0f, IM_COL32(255, 255, 255, 255), 0, 1.5f);
    }
}