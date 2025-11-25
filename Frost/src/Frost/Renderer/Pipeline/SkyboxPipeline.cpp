#include "SkyboxPipeline.h"
#include "Frost/Core/Application.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/Format.h"
#include "Frost/Renderer/Vertex.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Utils/Math/Transform.h"

#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"
#include "Frost/Renderer/DX11/TextureDX11.h"

namespace Frost
{
    struct alignas(16) VS_SkyboxConstants
    {
        Math::Matrix4x4 ViewMatrix;
        Math::Matrix4x4 ProjectionMatrix;
    };

    SkyboxPipeline::SkyboxPipeline()
    {
        Initialize();
        RendererAPI::GetRenderer()->RegisterPipeline(this);
    }

    SkyboxPipeline::~SkyboxPipeline()
    {
        Shutdown();
        RendererAPI::GetRenderer()->UnregisterPipeline(this);
    }

    void SkyboxPipeline::Initialize()
    {
        // Shaders
        ShaderDesc vsDesc = { .type = ShaderType::Vertex, .debugName = "VS_Skybox", .filePath = "../Frost/resources/shaders/VS_Skybox.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel, .debugName = "PS_Skybox", .filePath = "../Frost/resources/shaders/PS_Skybox.hlsl" };
        _skyboxVertexShader = Shader::Create(vsDesc);
        _skyboxPixelShader = Shader::Create(psDesc);

        // Input Layout
        const uint32_t stride = sizeof(Math::Vector3);
        InputLayout::VertexAttributeArray attributes = {
            {.name = "POSITION", .format = Format::RGB32_FLOAT, .arraySize = 1, .bufferIndex = 0, .offset = 0,  .elementStride = stride, .isInstanced = false }
        };
        _skyboxInputLayout = std::make_unique<InputLayoutDX11>(attributes, *_skyboxVertexShader);

        // Sampler
        SamplerConfig samplerConfig = { .filter = Filter::MIN_MAG_MIP_LINEAR, .addressU = AddressMode::CLAMP, .addressV = AddressMode::CLAMP, .addressW = AddressMode::CLAMP };
        _skyboxSampler = std::make_unique<SamplerDX11>(samplerConfig);

        // Constant Buffer
        auto* renderer = RendererAPI::GetRenderer();
        _vsSkyboxConstants = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(VS_SkyboxConstants), .dynamic = true });

        // Create cube geomtry
        CreateCubeMesh();
    }

    void SkyboxPipeline::Shutdown()
    {
        _vsSkyboxConstants.reset();
        _cubeIndexBuffer.reset();
        _cubeVertexBuffer.reset();
        _skyboxSampler.reset();
        _skyboxInputLayout.reset();
        _skyboxPixelShader.reset();
        _skyboxVertexShader.reset();
    }

    void SkyboxPipeline::CreateCubeMesh()
    {
        Math::Vector3 vertices[] = {
            { -1.0f, -1.0f,  1.0f }, {  1.0f, -1.0f,  1.0f },
            {  1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f },
            { -1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f },
            {  1.0f,  1.0f, -1.0f }, { -1.0f,  1.0f, -1.0f }
        };

        uint32_t indices[] = {
            0, 1, 2, 2, 3, 0,
            4, 7, 6, 6, 5, 4,
            0, 3, 7, 7, 4, 0,
            1, 5, 6, 6, 2, 1,
            3, 2, 6, 6, 7, 3,
            0, 4, 5, 5, 1, 0
        };
        _cubeIndexCount = ARRAYSIZE(indices);

        auto* renderer = RendererAPI::GetRenderer();
        _cubeVertexBuffer = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::VERTEX_BUFFER, .size = sizeof(vertices) }, vertices);
        _cubeIndexBuffer = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::INDEX_BUFFER, .size = sizeof(indices) }, indices);
    }

    void SkyboxPipeline::Render(CommandList* commandList, Texture* renderTarget, Texture* gbufferDepth, Texture* skyboxTexture, const Component::Camera& camera, const Component::WorldTransform& cameraTransform)
    {
		// Check if texture is loaded
        if (!skyboxTexture || !gbufferDepth || !renderTarget || !commandList) return;

        commandList->SetRenderTargets(1, &renderTarget, gbufferDepth);

        const float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
        const float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());
        const float viewportX = camera.viewport.x * windowWidth;
        const float viewportY = camera.viewport.y * windowHeight;
        const float viewportWidth = camera.viewport.width * windowWidth;
        const float viewportHeight = camera.viewport.height * windowHeight;
        commandList->SetViewport(viewportX, viewportY, viewportWidth, viewportHeight, 0.0f, 1.0f);

		// Draw inside of the cube
        commandList->SetRasterizerState(RasterizerMode::SolidCullNone);
        commandList->SetDepthStencilState(DepthMode::ReadOnly);

        // Update Constant Buffer
        VS_SkyboxConstants vsConstants;
        vsConstants.ViewMatrix = Math::Matrix4x4::CreateTranspose(Math::GetViewMatrix(cameraTransform));
        const float aspectRatio = (viewportHeight > 0) ? (viewportWidth / viewportHeight) : 1.0f;
        vsConstants.ProjectionMatrix = Math::Matrix4x4::CreateTranspose(Math::GetProjectionMatrix(camera, aspectRatio));
        _vsSkyboxConstants->UpdateData(commandList, &vsConstants, sizeof(VS_SkyboxConstants));

        commandList->SetShader(_skyboxVertexShader.get());
        commandList->SetShader(_skyboxPixelShader.get());
        commandList->SetInputLayout(_skyboxInputLayout.get());

        commandList->SetConstantBuffer(_vsSkyboxConstants.get(), 0);
        commandList->SetTexture(skyboxTexture, 0);
        commandList->SetSampler(_skyboxSampler.get(), 0);

        commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        commandList->SetVertexBuffer(_cubeVertexBuffer.get(), sizeof(Math::Vector3), 0);
        commandList->SetIndexBuffer(_cubeIndexBuffer.get(), 0);

        commandList->DrawIndexed(_cubeIndexCount, 0, 0);

        commandList->SetRasterizerState(RasterizerMode::Solid);
        commandList->SetDepthStencilState(DepthMode::ReadWrite);
    }
}