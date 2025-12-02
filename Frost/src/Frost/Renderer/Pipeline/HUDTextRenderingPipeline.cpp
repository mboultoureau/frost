#include "Frost/Renderer/Pipeline/HUDTextRenderingPipeline.h"

#include "Frost/Core/Application.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/InputLayout.h"

#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Asset/Font.h"

namespace Frost
{
    struct HUD_Vertex
    {
        float x, y, z;
        float u, v;
    };

    HUDTextRenderingPipeline::HUDTextRenderingPipeline()
    {
        RendererAPI::GetRenderer()->RegisterPipeline(this);

        Initialize();
    }

    HUDTextRenderingPipeline::~HUDTextRenderingPipeline()
    {
        RendererAPI::GetRenderer()->UnregisterPipeline(this);

        Shutdown();
    }

    void HUDTextRenderingPipeline::Initialize()
    {
        _commandList = std::make_unique<CommandListDX11>();

        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .debugName = "VS_HUDText",
                              .filePath = "../Frost/resources/shaders/VS_HUDText.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .debugName = "PS_HUDText",
                              .filePath = "../Frost/resources/shaders/PS_HUDText.hlsl" };
        _vertexShader = Shader::Create(vsDesc);
        _pixelShader = Shader::Create(psDesc);

        const uint32_t stride = sizeof(HUD_Vertex);
        InputLayout::VertexAttributeArray attributes = {
            { .name = "POSITION", .format = Format::RGB32_FLOAT, .offset = 0, .elementStride = stride },
            { .name = "TEXCOORD", .format = Format::RG32_FLOAT, .offset = 12, .elementStride = stride },
        };
        _inputLayout = std::make_unique<InputLayoutDX11>(attributes, *_vertexShader);

        auto* renderer = RendererAPI::GetRenderer();

        const uint32_t initialVBSize = 1024 * 1024;
        _vertexBuffer =
            renderer->CreateBuffer({ .usage = BufferUsage::VERTEX_BUFFER, .size = initialVBSize, .dynamic = true });

        _constantBuffer = renderer->CreateBuffer(
            { .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(HUDShaderParameters), .dynamic = true });

        _samplerPoint = std::make_unique<SamplerDX11>(SamplerConfig{ .filter = Filter::MIN_MAG_MIP_POINT });
        _samplerLinear = std::make_unique<SamplerDX11>(SamplerConfig{ .filter = Filter::MIN_MAG_MIP_LINEAR });
        _samplerAnisotropic = std::make_unique<SamplerDX11>(SamplerConfig{ .filter = Filter::ANISOTROPIC });
    }

    void HUDTextRenderingPipeline::Shutdown()
    {
        if (_commandList)
        {
            _commandList.reset();
        }
    }

    void HUDTextRenderingPipeline::BeginFrame()
    {
        if (!_enabled)
        {
            _commandList->BeginRecording();
            _enabled = true;

            _accumulatedVertices.clear();
            _drawCalls.clear();

            Texture* backBuffer = RendererAPI::GetRenderer()->GetBackBuffer();
            _commandList->SetRenderTargets(1, &backBuffer, nullptr);

            float screenWidth = (float)Application::GetWindow()->GetWidth();
            float screenHeight = (float)Application::GetWindow()->GetHeight();

            _commandList->SetViewport(0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 1.0f);

            _commandList->SetRasterizerState(RasterizerMode::SolidCullNone);
            _commandList->SetBlendState(BlendMode::Alpha);
            _commandList->SetDepthStencilState(DepthMode::None);
            _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

            _commandList->SetShader(_vertexShader.get());
            _commandList->SetShader(_pixelShader.get());
            _commandList->SetInputLayout(_inputLayout.get());

            _commandList->SetConstantBuffer(_constantBuffer.get(), 0);
        }
    }

    void HUDTextRenderingPipeline::Submit(const Component::HUDText& texte)
    {
        if (!_enabled)
            return;

        _RegenerateMesh(texte);
    }

    void HUDTextRenderingPipeline::EndFrame()
    {
        if (!_enabled)
            return;

        if (!_accumulatedVertices.empty())
        {
            uint32_t totalSize = (uint32_t)_accumulatedVertices.size() * sizeof(HUD_Vertex);

            if (totalSize > _vertexBuffer->GetSize())
            {
                FT_ENGINE_WARN("HUDText buffer overflow detected. Resizing from {} to {}",
                               _vertexBuffer->GetSize(),
                               totalSize * 2);

                auto* renderer = RendererAPI::GetRenderer();
                _vertexBuffer = renderer->CreateBuffer(
                    { .usage = BufferUsage::VERTEX_BUFFER, .size = totalSize * 2, .dynamic = true });
            }

            _vertexBuffer->UpdateData(_commandList.get(), _accumulatedVertices.data(), totalSize);

            const uint32_t stride = sizeof(HUD_Vertex);
            _commandList->SetVertexBuffer(_vertexBuffer.get(), stride, 0);

            for (const auto& drawCall : _drawCalls)
            {
                _constantBuffer->UpdateData(_commandList.get(), &drawCall.params, sizeof(drawCall.params));

                SetFilter(drawCall.filter);
                _commandList->SetTexture(drawCall.texture, 0);

                _commandList->Draw(drawCall.vertexCount, drawCall.vertexOffset);
            }
        }

        _commandList->SetRasterizerState(RasterizerMode::Solid);
        _commandList->SetBlendState(BlendMode::Alpha);
        _commandList->SetDepthStencilState(DepthMode::ReadWrite);

        _commandList->EndRecording();
        _commandList->Execute();

        RendererAPI::GetRenderer()->RestoreBackBufferRenderTarget();
        _enabled = false;
    }

    void HUDTextRenderingPipeline::SetFilter(Material::FilterMode filterMode)
    {
        switch (filterMode)
        {
            case Material::FilterMode::POINT:
                _commandList->SetSampler(_samplerPoint.get(), 0);
                break;
            case Material::FilterMode::LINEAR:
                _commandList->SetSampler(_samplerLinear.get(), 0);
                break;
            case Material::FilterMode::ANISOTROPIC:
                _commandList->SetSampler(_samplerAnisotropic.get(), 0);
                break;
        }
    }

    void HUDTextRenderingPipeline::_RegenerateMesh(const Component::HUDText& textComponent)
    {
        if (textComponent.text.empty() || !textComponent.font)
            return;

        const auto& font = textComponent.font;

        const float FONT_BAKE_SIZE = 32.0f;
        float screenWidth = (float)Application::GetWindow()->GetWidth();
        float screenHeight = (float)Application::GetWindow()->GetHeight();

        float startX_pixels = textComponent.viewport.x * screenWidth;
        float startY_pixels = textComponent.viewport.y * screenHeight;

        float currentX = startX_pixels;
        float currentY = startY_pixels + FONT_BAKE_SIZE * textComponent.fontSize;

        float z = 0.0f;
        float scale = textComponent.fontSize;

        DrawCall currentDrawCall;
        currentDrawCall.texture = font->GetAtlasTexture().get();
        currentDrawCall.filter = font->GetFilterMode();
        currentDrawCall.vertexOffset = (uint32_t)_accumulatedVertices.size();

        for (char c : textComponent.text)
        {
            if (c == ' ')
            {
                if (font->GetCharacterMetric(' ').advance > 0.0f)
                {
                    currentX += font->GetCharacterMetric(' ').advance * scale;
                }
                else
                {
                    currentX += scale * 0.3f;
                }
                continue;
            }

            const CharacterMetric& metric = font->GetCharacterMetric(c);

            if (metric.sizeX == 0 || metric.sizeY == 0)
                continue;

            float xpos_tl = currentX + metric.offsetX * scale;
            float ypos_tl = currentY + metric.offsetY * scale;

            float w = metric.sizeX * scale;
            float h = metric.sizeY * scale;

            float xpos_br = xpos_tl + w;
            float ypos_br = ypos_tl + h;

            float u0 = metric.uvX;
            float v0 = metric.uvY;
            float u1 = metric.uvX + metric.uvW;
            float v1 = metric.uvY + metric.uvH;

            _accumulatedVertices.push_back({ xpos_tl, ypos_tl, z, u0, v0 }); // TL
            _accumulatedVertices.push_back({ xpos_tl, ypos_br, z, u0, v1 }); // BL
            _accumulatedVertices.push_back({ xpos_br, ypos_tl, z, u1, v0 }); // TR

            _accumulatedVertices.push_back({ xpos_br, ypos_tl, z, u1, v0 }); // TR
            _accumulatedVertices.push_back({ xpos_tl, ypos_br, z, u0, v1 }); // BL
            _accumulatedVertices.push_back({ xpos_br, ypos_br, z, u1, v1 }); // BR

            currentX += metric.advance * scale;
        }

        currentDrawCall.vertexCount = (uint32_t)_accumulatedVertices.size() - currentDrawCall.vertexOffset;

        if (currentDrawCall.vertexCount > 0)
        {

            currentDrawCall.params.viewport[0] = textComponent.viewport.x;
            currentDrawCall.params.viewport[1] = textComponent.viewport.y;
            currentDrawCall.params.viewport[2] = screenWidth;
            currentDrawCall.params.viewport[3] = screenHeight;

            currentDrawCall.params.color[0] = textComponent.color.x;
            currentDrawCall.params.color[1] = textComponent.color.y;
            currentDrawCall.params.color[2] = textComponent.color.z;
            currentDrawCall.params.color[3] = textComponent.color.w;

            _drawCalls.push_back(std::move(currentDrawCall));
        }
    }
} // namespace Frost