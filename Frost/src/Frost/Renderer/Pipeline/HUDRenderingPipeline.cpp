#include "Frost/Renderer/Pipeline/HUDRenderingPipeline.h"

#include "Frost/Core/Application.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/InputLayout.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/Shader.h"

#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"

namespace Frost
{
    struct HUD_Vertex
    {
        float x, y, z;
        float u, v;
    };

    HUDRenderingPipeline::HUDRenderingPipeline()
    {
        RendererAPI::GetRenderer()->RegisterPipeline(this);

        Initialize();
    }

    HUDRenderingPipeline::~HUDRenderingPipeline()
    {
        RendererAPI::GetRenderer()->UnregisterPipeline(this);

        Shutdown();
    }

    void HUDRenderingPipeline::Initialize()
    {
#ifdef FT_PLATFORM_WINDOWS
        _commandList = std::make_unique<CommandListDX11>();
#else
        //_commandList = Renderer::GetDevice()->CreateCommandList();
#endif

        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .debugName = "VS_HUD",
                              .filePath = "../Frost/resources/shaders/VS_HUD.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .debugName = "PS_HUD",
                              .filePath = "../Frost/resources/shaders/PS_HUD.hlsl" };
        _vertexShader = Shader::Create(vsDesc);
        _pixelShader = Shader::Create(psDesc);

        HUD_Vertex vertices[] = {
            { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f }, // 0: Bottom-Left
            { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f }, // 1: Bottom-Right
            { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, // 2: Top-Left
            { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f }  // 3: Top-Right
        };
        uint32_t indices[] = { 2, 3, 1, 2, 1, 0 };
        _indexCount = sizeof(indices) / sizeof(uint32_t);

        auto* renderer = RendererAPI::GetRenderer();
        _vertexBuffer =
            renderer->CreateBuffer({ .usage = BufferUsage::VERTEX_BUFFER, .size = sizeof(vertices) }, vertices);
        _indexBuffer = renderer->CreateBuffer({ .usage = BufferUsage::INDEX_BUFFER, .size = sizeof(indices) }, indices);

        const uint32_t stride = sizeof(HUD_Vertex);
        InputLayout::VertexAttributeArray attributes = {
            { .name = "POSITION", .format = Format::RGB32_FLOAT, .offset = 0, .elementStride = stride },
            { .name = "TEXCOORD", .format = Format::RG32_FLOAT, .offset = 12, .elementStride = stride },
        };
        _inputLayout = std::make_unique<InputLayoutDX11>(attributes, *_vertexShader);

        _constantBuffer = renderer->CreateBuffer(
            { .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(HUDShaderParameters), .dynamic = true });

        _samplerPoint = std::make_unique<SamplerDX11>(SamplerConfig{ .filter = Filter::MIN_MAG_MIP_POINT });
        _samplerLinear = std::make_unique<SamplerDX11>(SamplerConfig{ .filter = Filter::MIN_MAG_MIP_LINEAR });
        _samplerAnisotropic = std::make_unique<SamplerDX11>(SamplerConfig{ .filter = Filter::ANISOTROPIC });
    }

    void HUDRenderingPipeline::Shutdown()
    {
        _samplerAnisotropic.reset();
        _samplerLinear.reset();
        _samplerPoint.reset();

        _constantBuffer.reset();
        _indexBuffer.reset();
        _vertexBuffer.reset();

        _inputLayout.reset();
        _pixelShader.reset();
        _vertexShader.reset();

        _commandList.reset();
    }

    void HUDRenderingPipeline::BeginFrame()
    {
        _enabled = true;
        _commandList->BeginRecording();

        Texture* backBuffer = RendererAPI::GetRenderer()->GetBackBuffer();
        _commandList->SetRenderTargets(1, &backBuffer, nullptr);

        float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
        float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());
        _commandList->SetViewport(0.0f, 0.0f, windowWidth, windowHeight, 0.0f, 1.0f);
        _commandList->SetScissorRect(0, 0, static_cast<int>(windowWidth), static_cast<int>(windowHeight));

        _commandList->SetRasterizerState(RasterizerMode::SolidCullNone);
        _commandList->SetBlendState(BlendMode::Alpha);
        _commandList->SetDepthStencilState(DepthMode::None);

        _commandList->SetShader(_vertexShader.get());
        _commandList->SetShader(_pixelShader.get());
        _commandList->SetInputLayout(_inputLayout.get());
        _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        _commandList->SetVertexBuffer(_vertexBuffer.get(), sizeof(HUD_Vertex), 0);
        _commandList->SetIndexBuffer(_indexBuffer.get(), 0);
        _commandList->SetConstantBuffer(_constantBuffer.get(), 0);
    }

    void HUDRenderingPipeline::Submit(const Component::HUDImage& image)
    {
        float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
        float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());

        HUDShaderParameters params = {};
        params.viewport[0] = image.viewport.x;
        params.viewport[1] = image.viewport.y;
        params.viewport[2] = image.viewport.width;
        params.viewport[3] = image.viewport.height;

        _constantBuffer->UpdateData(_commandList.get(), &params, sizeof(params));

        SetFilter(image.textureFilter);
        _commandList->SetTexture(image.texture.get(), 0);
        _commandList->DrawIndexed(_indexCount, 0, 0);
    }

    void HUDRenderingPipeline::Submit(const Component::UIButton& button)
    {
        Submit(static_cast<const Component::HUDImage&>(button));
    }

    void HUDRenderingPipeline::EndFrame()
    {
        if (!_enabled)
            return;

        _commandList->SetRasterizerState(RasterizerMode::Solid);
        _commandList->SetBlendState(BlendMode::None);
        _commandList->SetDepthStencilState(DepthMode::ReadWrite);

        _commandList->EndRecording();
        _commandList->Execute();

        RendererAPI::GetRenderer()->RestoreBackBufferRenderTarget();
        _enabled = false;
    }

    void HUDRenderingPipeline::SetFilter(Material::FilterMode filterMode)
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
            default:
                _commandList->SetSampler(_samplerAnisotropic.get(), 0);
                break;
        }
    }
} // namespace Frost