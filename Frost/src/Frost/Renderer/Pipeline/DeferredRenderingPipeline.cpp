#include "Frost/Renderer/Pipeline/DeferredRenderingPipeline.h"
#include "Frost/Core/Application.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/Format.h"
#include "Frost/Renderer/GraphicsTypes.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Asset/Model.h"
#include "Frost/Utils/Math/Vector.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Renderer/Vertex.h"

#ifdef FT_DEBUG
#include "Frost/Debugging/DebugInterface/DebugRendering.h"
#endif

#ifdef FT_PLATFORM_WINDOWS
#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#include "Frost/Renderer/DX11/BufferDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"
#endif

#include <array>

namespace Frost
{
    // Lighting shader
    constexpr int MAX_DIRECTIONAL_LIGHTS = 2;
    constexpr int MAX_POINT_LIGHTS = 100;
    constexpr int MAX_SPOT_LIGHTS = 50;

    struct alignas(16) LightConstants
    {
        Math::Vector3           CameraPosition;
        float                   Padding0;

        struct alignas(16) DirectionalLightData
        {
            Math::Vector3       Direction;
            float               Padding1;
            Math::Vector3       Color;
            float               Intensity;
        };

        struct alignas(16) PointLightData
        {
            Math::Vector3       Position;
            float               Radius;
            Math::Vector3       Color;
            float               Intensity;
        };

        struct alignas(16) SpotLightData
        {
            Math::Vector3       Position;
            float               Radius;
            Math::Vector3       Direction;
            float               Intensity;
            Math::Vector3       Color;
            float               InnerConeAngle; // cos(angle)
            float               OuterConeAngle; // cos(angle)
            float               Padding2[3];
        };

        DirectionalLightData    DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
        PointLightData          PointLights[MAX_POINT_LIGHTS];
        SpotLightData           SpotLights[MAX_SPOT_LIGHTS];

        int                     NumDirectionalLights;
        int                     NumPointLights;
        int                     NumSpotLights;
        float                   Padding3;
    };

	// GBuffer shaders
    struct alignas(16) VS_PerFrameConstants
    {
        Math::Matrix4x4         ViewMatrix;
        Math::Matrix4x4         ProjectionMatrix;
    };

    struct alignas(16) VS_PerObjectConstants
    {
        Math::Matrix4x4         World;
    };

    struct alignas(16) PS_MaterialConstants
    {
        Math::Vector2           UVTiling;
        Math::Vector2           UVOffset;
    };


    DeferredRenderingPipeline::DeferredRenderingPipeline()
    {
        Initialize();

		RendererAPI::GetRenderer()->RegisterPipeline(this);
    }

    DeferredRenderingPipeline::~DeferredRenderingPipeline()
    {
        Shutdown();

		RendererAPI::GetRenderer()->UnregisterPipeline(this);
    }

    void DeferredRenderingPipeline::Initialize()
    {
#ifdef FT_PLATFORM_WINDOWS
        _commandList = std::make_unique<CommandListDX11>();
#else
        //_commandList = Renderer::GetDevice()->CreateCommandList();
#endif

        uint32_t width = Application::GetWindow()->GetWidth();
        uint32_t height = Application::GetWindow()->GetHeight();

        if (width == 0 || height == 0) return;

        _CreateGBufferTextures(width, height);

#ifdef FT_PLATFORM_WINDOWS
        ShaderDesc gBufferVSDesc = { .type = ShaderType::Vertex, .debugName = "VS_GBuffer", .filePath = "../Frost/resources/shaders/VS_GBuffer.hlsl" };
        ShaderDesc gBufferPSDesc = { .type = ShaderType::Pixel, .debugName = "PS_GBuffer", .filePath = "../Frost/resources/shaders/PS_GBuffer.hlsl" };
        _gBufferVertexShader = std::make_unique<ShaderDX11>(gBufferVSDesc);
        _gBufferPixelShader = std::make_unique<ShaderDX11>(gBufferPSDesc);

        ShaderDesc lightingVSDesc = { .type = ShaderType::Vertex, .debugName = "VS_Lighting", .filePath = "../Frost/resources/shaders/VS_Lighting.hlsl" };
        ShaderDesc lightingPSDesc = { .type = ShaderType::Pixel, .debugName = "PS_Lighting", .filePath = "../Frost/resources/shaders/PS_Lighting.hlsl" };
        _lightingVertexShader = std::make_unique<ShaderDX11>(lightingVSDesc);
        _lightingPixelShader = std::make_unique<ShaderDX11>(lightingPSDesc);
#else
#error "DeferredRendering only supports Windows platform in this implementation."
#endif

        const uint32_t gBufferVertexStride = sizeof(Vertex);
        InputLayout::VertexAttributeArray gBufferAttributes = {
            {.name = "POSITION", .format = Format::RGB32_FLOAT, .arraySize = 1, .bufferIndex = 0, .offset = 0,  .elementStride = gBufferVertexStride, .isInstanced = false },
            {.name = "NORMAL",   .format = Format::RGB32_FLOAT,  .arraySize = 1, .bufferIndex = 0, .offset = 12, .elementStride = gBufferVertexStride, .isInstanced = false },
            {.name = "TEXCOORD", .format = Format::RG32_FLOAT,   .arraySize = 1, .bufferIndex = 0, .offset = 24, .elementStride = gBufferVertexStride, .isInstanced = false },
            {.name = "TANGENT",  .format = Format::RGBA32_FLOAT, .arraySize = 1, .bufferIndex = 0, .offset = 32, .elementStride = gBufferVertexStride, .isInstanced = false },
        };
        _gBufferInputLayout = std::make_unique<InputLayoutDX11>(gBufferAttributes, *_gBufferVertexShader);
        _lightingInputLayout = nullptr;

        SamplerConfig materialSamplerConfig = { .filter = Filter::MIN_MAG_MIP_LINEAR, .addressU = AddressMode::WRAP, .addressV = AddressMode::WRAP, .addressW = AddressMode::WRAP };
        _materialSampler = std::make_unique<SamplerDX11>(materialSamplerConfig);
        SamplerConfig gBufferSamplerConfig = { .filter = Filter::MIN_MAG_MIP_POINT, .addressU = AddressMode::CLAMP, .addressV = AddressMode::CLAMP, .addressW = AddressMode::CLAMP };
        _gBufferSampler = std::make_unique<SamplerDX11>(gBufferSamplerConfig);

        auto* renderer = RendererAPI::GetRenderer();
        _vsPerFrameConstants = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(VS_PerFrameConstants), .dynamic = true });
        _vsPerObjectConstants = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(VS_PerObjectConstants), .dynamic = true });
        _lightConstantsBuffer = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(LightConstants), .dynamic = true });
        _psMaterialConstants = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER, .size = sizeof(PS_MaterialConstants), .dynamic = true });

        CreateDefaultTextures();
    }

    void DeferredRenderingPipeline::Shutdown()
    {
        _lightConstantsBuffer.reset();
        _vsPerObjectConstants.reset();
        _vsPerFrameConstants.reset();
		_psMaterialConstants.reset();

        _gBufferSampler.reset();
        _materialSampler.reset();

        _lightingInputLayout.reset();
        _gBufferInputLayout.reset();

        _lightingPixelShader.reset();
        _lightingVertexShader.reset();
        _gBufferPixelShader.reset();
        _gBufferVertexShader.reset();

        _depthStencilTexture.reset();
        _materialTexture.reset();
        _worldPositionTexture.reset();
        _normalTexture.reset();
        _albedoTexture.reset();

        _defaultNormalTexture.reset();
        _defaultAlbedoTexture.reset();
        _defaultMetallicTexture.reset();
		_defaultRoughnessTexture.reset();
		_defaultAOTexture.reset();

        _commandList.reset();
    }


    void DeferredRenderingPipeline::CreateDefaultTextures()
    {
#ifdef FT_PLATFORM_WINDOWS
        uint8_t whitePixel[4] = { 255, 255, 255, 255 };
        _defaultAlbedoTexture = std::make_unique<TextureDX11>(1, 1, Format::RGBA8_UNORM, whitePixel, "DefaultAlbedoTexture");

        uint8_t flatNormalPixel[4] = { 128, 128, 255, 255 };
        _defaultNormalTexture = std::make_unique<TextureDX11>(1, 1, Format::RGBA8_UNORM, flatNormalPixel, "DefaultNormalTexture");

		uint8_t defaultMetallicPixel[4] = { 0, 255, 255, 255 };
        _defaultMetallicTexture = std::make_unique<TextureDX11>(1, 1, Format::RGBA8_UNORM, defaultMetallicPixel, "DefaultMetallicTexture");

		uint8_t defaultRoughnessPixel[4] = { 255, 255, 255, 255 };
		_defaultRoughnessTexture = std::make_unique<TextureDX11>(1, 1, Format::RGBA8_UNORM, defaultRoughnessPixel, "DefaultRoughnessTexture");

		uint8_t defaultAOPixel[4] = { 255, 255, 255, 255 };
		_defaultAOTexture = std::make_unique<TextureDX11>(1, 1, Format::RGBA8_UNORM, defaultAOPixel, "DefaultAOTexture");
#endif
    }

    void DeferredRenderingPipeline::_CreateGBufferTextures(uint32_t width, uint32_t height)
    {
        _albedoTexture.reset();
        _normalTexture.reset();
        _worldPositionTexture.reset();
        _materialTexture.reset();
        _depthStencilTexture.reset();

        TextureConfig textureConfig = { .width = width, .height = height, .isRenderTarget = true, .isShaderResource = true, .hasMipmaps = false };

        textureConfig.format = Format::RGBA8_UNORM;
        _albedoTexture = std::make_unique<TextureDX11>(textureConfig);

        textureConfig.format = Format::RGBA16_FLOAT;
        _normalTexture = std::make_unique<TextureDX11>(textureConfig);

        textureConfig.format = Format::RGBA32_FLOAT;
        _worldPositionTexture = std::make_unique<TextureDX11>(textureConfig);

        textureConfig.format = Format::RG8_UNORM;
        _materialTexture = std::make_unique<TextureDX11>(textureConfig);

        TextureConfig depthConfig = { .format = Format::D24_UNORM_S8_UINT, .width = width, .height = height, .isRenderTarget = true, .isShaderResource = false, .hasMipmaps = false };
        _depthStencilTexture = std::make_unique<TextureDX11>(depthConfig);
    }

    void DeferredRenderingPipeline::BeginFrame(const Component::Camera& camera, const Component::WorldTransform& cameraTransform)
    {
        if (!_albedoTexture) return;
        _enabled = true;

        _commandList->BeginRecording();

#ifdef FT_DEBUG
        if (Debug::RendererConfig::wireframeMode)
        {
            _commandList->SetRasterizerState(RasterizerMode::Wireframe);
		}
#endif

        const float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
        const float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());

        const float viewportX = camera.viewport.x * windowWidth;
        const float viewportY = camera.viewport.y * windowHeight;
        const float viewportWidth = camera.viewport.width * windowWidth;
        const float viewportHeight = camera.viewport.height * windowHeight;

        const float aspectRatio = (viewportHeight > 0) ? (viewportWidth / viewportHeight) : 1.0f;

        if (viewportWidth == 0 || viewportHeight == 0)
        {
            FT_ENGINE_ERROR("Viewport width and height should be > 0");
            _enabled = false;
            return;
        }

        VS_PerFrameConstants vsPerFrameData;
        vsPerFrameData.ViewMatrix = Math::Matrix4x4::CreateTranspose(Math::GetViewMatrix(cameraTransform));
        vsPerFrameData.ProjectionMatrix = Math::Matrix4x4::CreateTranspose(Math::GetProjectionMatrix(camera, aspectRatio));
        _vsPerFrameConstants->UpdateData(_commandList.get(), &vsPerFrameData, sizeof(VS_PerFrameConstants));

        Texture* gBufferRTs[] = {
            _albedoTexture.get(), _normalTexture.get(), _worldPositionTexture.get(), _materialTexture.get()
        };
        _commandList->SetRenderTargets(ARRAYSIZE(gBufferRTs), gBufferRTs, _depthStencilTexture.get());


        _commandList->SetViewport(viewportX, viewportY, viewportWidth, viewportHeight, 0.0f, 1.0f);

        _commandList->SetScissorRect(
            static_cast<int>(viewportX),
            static_cast<int>(viewportY),
            static_cast<int>(viewportWidth),
            static_cast<int>(viewportHeight)
        );

        if (camera.clearOnRender)
        {
            const float clearColor[4] = { camera.backgroundColor.r, camera.backgroundColor.g, camera.backgroundColor.b, camera.backgroundColor.a };
            for (Texture* rt : gBufferRTs)
            {
                _commandList->ClearRenderTarget(rt, clearColor, camera.viewport);
            }
        }

        _commandList->ClearDepthStencil(_depthStencilTexture.get(), true, 1.0f, true, 0);

        _commandList->SetShader(_gBufferVertexShader.get());
        _commandList->SetShader(_gBufferPixelShader.get());
        _commandList->SetInputLayout(_gBufferInputLayout.get());

        _commandList->SetConstantBuffer(_vsPerFrameConstants.get(), 0);
    }

    void DeferredRenderingPipeline::SubmitModel(const Model& model, const Math::Matrix4x4& worldMatrix)
    {
        if (!_albedoTexture) return;
        if (!_enabled) return;

        VS_PerObjectConstants vsPerObjectData;
        vsPerObjectData.World = Math::Matrix4x4::CreateTranspose(worldMatrix);
        _vsPerObjectConstants->UpdateData(_commandList.get(), &vsPerObjectData, sizeof(VS_PerObjectConstants));
        _commandList->SetConstantBuffer(_vsPerObjectConstants.get(), 1);

        _commandList->SetSampler(_materialSampler.get(), 0);

        _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);

        for (const auto& mesh : model.GetMeshes())
        {
			auto& material = model.GetMaterials()[mesh.GetMaterialIndex()];

            PS_MaterialConstants psMaterialData;
            psMaterialData.UVTiling = material.uvTiling;
            psMaterialData.UVOffset = material.uvOffset;
            _psMaterialConstants->UpdateData(_commandList.get(), &psMaterialData, sizeof(PS_MaterialConstants));
            _commandList->SetConstantBuffer(_psMaterialConstants.get(), 2);

            if (!material.albedoTextures.empty())
            {
                _commandList->SetTexture(material.albedoTextures[0].get(), 0);
            }
            else
            {
                _commandList->SetTexture(_defaultAlbedoTexture.get(), 0);
            }

            if (!material.normalTextures.empty())
            {
                _commandList->SetTexture(material.normalTextures[0].get(), 1);
            }
            else
            {
                _commandList->SetTexture(_defaultNormalTexture.get(), 1);
            }

            if (!material.metallicTextures.empty())
            {
                _commandList->SetTexture(material.metallicTextures[0].get(), 2);
            }
            else
            {
                _commandList->SetTexture(_defaultMetallicTexture.get(), 2);
            }

            if (!material.roughnessTextures.empty())
            {
                _commandList->SetTexture(material.roughnessTextures[0].get(), 3);
            }
            else
            {
                _commandList->SetTexture(_defaultRoughnessTexture.get(), 3);
            }

            if (!material.aoTextures.empty())
            {
                _commandList->SetTexture(material.aoTextures[0].get(), 4);
            }
            else
            {
                _commandList->SetTexture(_defaultAOTexture.get(), 4);
			}

            _commandList->SetVertexBuffer(mesh.GetVertexBuffer(), mesh.GetVertexStride(), 0);
            _commandList->SetIndexBuffer(mesh.GetIndexBuffer(), 0);
            _commandList->DrawIndexed(mesh.GetIndexCount(), 0, 0);
        }
    }

    void DeferredRenderingPipeline::EndFrame(const Component::Camera& camera, const Component::WorldTransform& cameraTransform, const std::vector<std::pair<Component::Light, Component::WorldTransform>>& lights)
    {
        if (!_albedoTexture) return;
        if (!_enabled) return;

        LightConstants lightData = {};
        lightData.CameraPosition = cameraTransform.position;

        for (const auto& lightPair : lights)
        {
            const auto& lightComponent = lightPair.first;
            const auto& lightTransform = lightPair.second;

            switch (lightComponent.type)
            {
            case Component::LightType::Directional:
            {
                if (lightData.NumDirectionalLights < MAX_DIRECTIONAL_LIGHTS)
                {
                    auto& light = lightData.DirectionalLights[lightData.NumDirectionalLights++];
                    light.Direction = lightTransform.GetForward();
                    light.Color = lightComponent.color;
                    light.Intensity = lightComponent.intensity;
                }
                break;
            }
            case Component::LightType::Point:
            {
                if (lightData.NumPointLights < MAX_POINT_LIGHTS)
                {
                    auto& light = lightData.PointLights[lightData.NumPointLights++];
                    light.Position = lightTransform.position;
                    light.Radius = lightComponent.radius;
                    light.Color = lightComponent.color;
                    light.Intensity = lightComponent.intensity;
                }
                break;
            }
            case Component::LightType::Spot:
            {
                if (lightData.NumSpotLights < MAX_SPOT_LIGHTS)
                {
                    auto& light = lightData.SpotLights[lightData.NumSpotLights++];
                    light.Position = lightTransform.position;
                    light.Radius = lightComponent.radius;
                    light.Direction = lightTransform.GetForward();
                    light.Color = lightComponent.color;
                    light.Intensity = lightComponent.intensity;
                    light.InnerConeAngle = cosf(lightComponent.innerConeAngle.value());
                    light.OuterConeAngle = cosf(lightComponent.outerConeAngle.value());
                }
                break;
            }
            }
        }
        _lightConstantsBuffer->UpdateData(_commandList.get(), &lightData, sizeof(LightConstants));

        Texture* backBuffer = RendererAPI::GetRenderer()->GetBackBuffer();
        _commandList->SetRenderTargets(1, &backBuffer, nullptr);

        const float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
        const float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());
        _commandList->SetViewport(0.0f, 0.0f, windowWidth, windowHeight, 0.0f, 1.0f);

        _commandList->SetShader(_lightingVertexShader.get());
        _commandList->SetShader(_lightingPixelShader.get());
        _commandList->SetInputLayout(nullptr);

        _commandList->SetTexture(_albedoTexture.get(), 0);
        _commandList->SetTexture(_normalTexture.get(), 1);
        _commandList->SetTexture(_worldPositionTexture.get(), 2);
        _commandList->SetTexture(_materialTexture.get(), 3);
        _commandList->SetSampler(_gBufferSampler.get(), 0);

        _commandList->SetConstantBuffer(_lightConstantsBuffer.get(), 0);

        _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        _commandList->Draw(3, 0);

#ifdef FT_DEBUG
        _commandList->SetRasterizerState(RasterizerMode::Solid);
#endif

        _commandList->SetScissorRect(0, 0, Application::GetWindow()->GetWidth(), Application::GetWindow()->GetHeight());

        _commandList->EndRecording();
        _commandList->Execute();

        RendererAPI::GetRenderer()->RestoreBackBufferRenderTarget();
    }
}