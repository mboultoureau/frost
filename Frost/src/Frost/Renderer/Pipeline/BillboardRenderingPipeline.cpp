/*
#include "Frost/Renderer/Pipeline/BillboardRenderingPipeline.h"

#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/DX11/CommandListDX11.h"

#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"

#include <array>

namespace Frost
{

    BillboardRenderingPipeline::BillboardRenderingPipeline()
    {
        RendererAPI::GetRenderer()->RegisterPipeline(this);
        Initialize();
    }

    BillboardRenderingPipeline::~BillboardRenderingPipeline()
    {
        RendererAPI::GetRenderer()->UnregisterPipeline(this);
        Shutdown();
    }

    void BillboardRenderingPipeline::Initialize()
    {
        Renderer* renderer = RendererAPI::GetRenderer();


        RendererDX11* dx11Renderer = static_cast<RendererDX11*>(renderer);
        ID3D11Device1* device = dx11Renderer->GetDevice();


#ifdef FT_PLATFORM_WINDOWS
        _commandList = std::make_unique<CommandListDX11>();
#endif

        ShaderDesc vsDesc;
        vsDesc.filePath = "../Frost/resources/shaders/VS_Billboard.hlsl";
        vsDesc.type = ShaderType::Vertex;
        vsDesc.debugName = "Billboard Vertex Shader";
        _vertexShader = Shader::Create(vsDesc);

        ShaderDesc psDesc;
        psDesc.filePath = "../Frost/resources/shaders/PS_Billboard.hlsl ";
        psDesc.type = ShaderType::Pixel;
        psDesc.debugName = "Billboard Pixel Shader";
        _pixelShader = Shader::Create(psDesc);


        BufferConfig perFrameConfig;
        perFrameConfig.usage = BufferUsage::CONSTANT_BUFFER;
        perFrameConfig.size = sizeof(VS_PerFrameConstants);
        perFrameConfig.dynamic = true;
        perFrameConfig.debugName = "Billboard Per Frame CB";
        _perFrameConstantBuffer = renderer->CreateBuffer(perFrameConfig);


        BufferConfig perDrawConfig;
        perDrawConfig.usage = BufferUsage::CONSTANT_BUFFER;
        perDrawConfig.size = sizeof(BillboardConstants);
        perDrawConfig.dynamic = true;
        perDrawConfig.debugName = "Billboard Per Draw CB";
        _perDrawConstantBuffer = renderer->CreateBuffer(perDrawConfig);


        SamplerConfig pointConfig;
        pointConfig.filter = Filter::MIN_MAG_MIP_POINT;

        _samplerPoint = std::make_shared<SamplerDX11>(pointConfig);


        SamplerConfig linearConfig;
        linearConfig.filter = Filter::MIN_MAG_MIP_LINEAR;
        _samplerLinear = std::make_shared<SamplerDX11>(linearConfig);
    }

    void BillboardRenderingPipeline::Shutdown()
    {
        _commandList.reset();
        _vertexShader.reset();
        _pixelShader.reset();
        _perFrameConstantBuffer.reset();
        _perDrawConstantBuffer.reset();
        _samplerPoint.reset();
        _samplerLinear.reset();
    }

    void BillboardRenderingPipeline::BeginFrame(const Frost::Math::Matrix4x4& view,
                                                const Frost::Math::Matrix4x4& projection,
                                                Texture* renderTarget,
                                                Texture* depthStencil)
    {

        _commandList->BeginRecording();

        Texture* targets[] = { renderTarget };
        _commandList->SetRenderTargets(1, targets, depthStencil);

        VS_PerFrameConstants constants = {};
        constants.ViewMatrix = view;
        constants.ProjectionMatrix = projection;

        _perFrameConstantBuffer->UpdateData(_commandList.get(), (const void*)&constants, sizeof(constants));

        _commandList->SetShader(_vertexShader.get());
        _commandList->SetShader(_pixelShader.get());

        _commandList->SetConstantBuffer(_perFrameConstantBuffer.get(), 0); // b0
        _commandList->SetConstantBuffer(_perDrawConstantBuffer.get(), 1);  // b1

        _commandList->SetRasterizerState(RasterizerMode::Solid);
        _commandList->SetBlendState(BlendMode::Alpha);
        _commandList->SetDepthStencilState(DepthMode::ReadWrite);
        _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

        _enabled = true;
    }

    void BillboardRenderingPipeline::Submit(const BillboardConstants& constants,
                                            std::shared_ptr<Texture> texture,
                                            Material::FilterMode textureFilter)
    {
        if (!_enabled)
            return;

        _perDrawConstantBuffer->UpdateData(_commandList.get(), (const void*)&constants, sizeof(constants));

        _SetFilter(textureFilter);
        _commandList->SetTexture(texture.get(), 0);

        _commandList->Draw(6, 0);
    }

    void BillboardRenderingPipeline::EndFrame()
    {
        if (!_enabled)
            return;

        _commandList->EndRecording();
        _commandList->Execute();

        RendererAPI::GetRenderer()->RestoreBackBufferRenderTarget();

        _enabled = false;
    }

    void BillboardRenderingPipeline::_SetFilter(Material::FilterMode filterMode)
    {
        switch (filterMode)
        {
            case Material::FilterMode::POINT:
                _commandList->SetSampler(_samplerPoint.get(), 0);
                break;
            case Material::FilterMode::LINEAR:
            default:
                _commandList->SetSampler(_samplerLinear.get(), 0);
                break;
        }
    }
} // namespace Frost*/