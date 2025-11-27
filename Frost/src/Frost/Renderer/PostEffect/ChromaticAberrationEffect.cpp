#include "Frost/Renderer/PostEffect/ChromaticAberrationEffect.h"
#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Renderer/RendererAPI.h"

#include "Frost/Renderer/DX11/TextureDX11.h"

#undef max
#undef min
#include <algorithm>
#include <imgui.h>

namespace Frost
{
    struct alignas(16) ChromaticAberrationConstants
    {
        Math::Vector2 centerRed;
        Math::Vector2 centerGreen;
        Math::Vector2 centerBlue;
        float strength;
    };

    ChromaticAberrationEffect::ChromaticAberrationEffect()
    {
#ifdef FT_PLATFORM_WINDOWS
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .debugName = "VS_ChromaticAberration",
                              .filePath = "../Frost/resources/shaders/PostEffect/"
                                          "VS_ChromaticAberration.hlsl" };
        _vertexShader = std::make_unique<ShaderDX11>(vsDesc);

        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .debugName = "PS_ChromaticAberration",
                              .filePath = "../Frost/resources/shaders/PostEffect/"
                                          "PS_ChromaticAberration.hlsl" };
        _pixelShader = std::make_unique<ShaderDX11>(psDesc);
#endif

        auto* renderer = RendererAPI::GetRenderer();
        _constantsBuffer = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER,
                                                                .size = sizeof(ChromaticAberrationConstants),
                                                                .dynamic = true,
                                                                .debugName = "CB_ChromaticAberration" });

        SamplerConfig samplerConfig = { .filter = Filter::MIN_MAG_MIP_LINEAR,
                                        .addressU = AddressMode::CLAMP,
                                        .addressV = AddressMode::CLAMP,
                                        .addressW = AddressMode::CLAMP };
        _sampler = std::make_unique<SamplerDX11>(samplerConfig);
    }

    void ChromaticAberrationEffect::OnPostRender(float deltaTime,
                                                 CommandList* commandList,
                                                 Texture* source,
                                                 Texture* destination)
    {
        ChromaticAberrationConstants constants;
        constants.centerRed = _centerRed;
        constants.centerGreen = _centerGreen;
        constants.centerBlue = _centerBlue;
        constants.strength = _strength;
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

    void ChromaticAberrationEffect::OnImGuiRender(float deltaTime)
    {
        // Settings
        ImGui::Text("Settings");
        ImGui::Separator();
        ImGui::SliderFloat("Strength", &_strength, 0.0f, 0.2f, "%.4f");
        ImGui::DragFloat2("Center Red", &_centerRed.x, 0.005f, 0.0f, 1.0f, "%.3f");
        ImGui::DragFloat2("Center Green", &_centerGreen.x, 0.005f, 0.0f, 1.0f, "%.3f");
        ImGui::DragFloat2("Center Blue", &_centerBlue.x, 0.005f, 0.0f, 1.0f, "%.3f");

        // Center visualization
        ImGui::Text("Center Control");
        ImGui::Separator();

        ImGui::Text("Edit Center:");
        ImGui::SameLine();
        ImGui::RadioButton("Red", &_activeCenter, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Green", &_activeCenter, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Blue", &_activeCenter, 2);

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
            float normalizedX = (mousePos.x - zoneTopLeft.x) / zoneSize.x;
            float normalizedY = (mousePos.y - zoneTopLeft.y) / zoneSize.y;

            normalizedX = std::max(0.0f, std::min(1.0f, normalizedX));
            normalizedY = std::max(0.0f, std::min(1.0f, normalizedY));

            switch (_activeCenter)
            {
                case 0: // Red
                    _centerRed = { normalizedX, normalizedY };
                    break;
                case 1: // Green
                    _centerGreen = { normalizedX, normalizedY };
                    break;
                case 2: // Blue
                    _centerBlue = { normalizedX, normalizedY };
                    break;
            }
        }

        ImVec2 pointPosR(zoneTopLeft.x + _centerRed.x * zoneSize.x, zoneTopLeft.y + _centerRed.y * zoneSize.y);
        ImVec2 pointPosG(zoneTopLeft.x + _centerGreen.x * zoneSize.x, zoneTopLeft.y + _centerGreen.y * zoneSize.y);
        ImVec2 pointPosB(zoneTopLeft.x + _centerBlue.x * zoneSize.x, zoneTopLeft.y + _centerBlue.y * zoneSize.y);

        ImVec2 selectedPos;
        if (_activeCenter == 0)
            selectedPos = pointPosR;
        else if (_activeCenter == 1)
            selectedPos = pointPosG;
        else
            selectedPos = pointPosB;
        drawList->AddCircle(selectedPos, 8.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);

        drawList->AddCircleFilled(pointPosR, 6.0f, IM_COL32(255, 50, 50, 255));
        drawList->AddCircleFilled(pointPosG, 6.0f, IM_COL32(50, 255, 50, 255));
        drawList->AddCircleFilled(pointPosB, 6.0f, IM_COL32(50, 50, 255, 255));
    }
} // namespace Frost