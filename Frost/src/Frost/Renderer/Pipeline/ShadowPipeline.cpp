#include "ShadowPipeline.h"
#include "Frost/Core/Application.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Format.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Sampler.h"
#include "Frost/Renderer/Vertex.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Scene.h"

#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"
#include "Frost/Renderer/DX11/TextureDX11.h"
#include <variant>

// windows.....
#undef max
#undef min

using namespace Frost::Math;

namespace Frost
{
    constexpr int MAX_DIRECTIONAL_LIGHTS = 2;
    constexpr int MAX_POINT_LIGHTS = 100;
    constexpr int MAX_SPOT_LIGHTS = 50;

    struct alignas(16) VS_ShadowConstants
    {
        DirectX::XMMATRIX World;
        DirectX::XMMATRIX LightViewProj;
    };

    struct alignas(16) DirectionalLightData : public LightData
    {
        Math::Vector3 CameraPosition;
        unsigned int shadowResolution;
        Math::Vector3 Direction;
        float Intensity;
        Math::Vector3 Color;
        float CascadeNear; // Add these
        Math::Vector3 CameraFwd;
        float CascadeFar; // Add these
        Math::Vector3 LightPosition;
        float Padding[2];
    };

    struct alignas(16) AmbiantLightData : public LightData
    {
        Math::Vector3 CameraPosition;
        float Intensity;
        Math::Vector3 Color;
    };

    struct alignas(16) PointLightData : public LightData
    {
        Math::Vector3 CameraPosition;
        unsigned int ShadowResolution;
        Math::Vector3 Position;
        float Radius;
        Math::Vector3 Color;
        float Intensity;
        Math::Vector3 LightDirection;
    };

    struct alignas(16) SpotLightData : public LightData
    {
        Math::Vector3 CameraPosition;
        float InnerConeAngle; // cos(angle)
        Math::Vector3 Position;
        float Radius;
        Math::Vector3 Direction;
        float Intensity;
        Math::Vector3 Color;
        float OuterConeAngle; // cos(angle)
        unsigned int shadowResolution;
    };

    ShadowPipeline::ShadowPipeline()
    {
        Initialize();
        RendererAPI::GetRenderer()->RegisterPipeline(this);
    }

    ShadowPipeline::~ShadowPipeline()
    {
        Shutdown();
        RendererAPI::GetRenderer()->UnregisterPipeline(this);
    }

    void ShadowPipeline::OnWindowResize(WindowResizeEvent& e)
    {
        Shutdown();
        Initialize();
    }

    void ShadowPipeline::InitLightShaders()
    {
        ShaderDesc vsDesc;
        ShaderDesc psDesc;

        // Ambiant light
        vsDesc = { .type = ShaderType::Vertex,
                   .debugName = "VS_AmbiantLighting",
                   .filePath = "../Frost/resources/shaders/Light/VS_AmbiantLight.hlsl" };
        psDesc = { .type = ShaderType::Pixel,
                   .debugName = "PS_AmbiantLighting",
                   .filePath = "../Frost/resources/shaders/Light/PS_AmbiantLight.hlsl" };
        _ambiantLightVertexShader = Shader::Create(vsDesc);
        _ambiantLightPixelShader = Shader::Create(psDesc);

        // Point light
        vsDesc = { .type = ShaderType::Vertex,
                   .debugName = "VS_PointLight",
                   .filePath = "../Frost/resources/shaders/Light/VS_PointLight.hlsl" };
        psDesc = { .type = ShaderType::Pixel,
                   .debugName = "PS_PointLight",
                   .filePath = "../Frost/resources/shaders/Light/PS_PointLight.hlsl" };
        _pointLightVertexShader = Shader::Create(vsDesc);
        _pointLightPixelShader = Shader::Create(psDesc);

        // Directional light
        vsDesc = { .type = ShaderType::Vertex,
                   .debugName = "VS_DirectionalLight",
                   .filePath = "../Frost/resources/shaders/Light/VS_DirectionalLight.hlsl" };
        psDesc = { .type = ShaderType::Pixel,
                   .debugName = "PS_DirectionalLight",
                   .filePath = "../Frost/resources/shaders/Light/PS_DirectionalLight.hlsl" };
        _directionalLightVertexShader = Shader::Create(vsDesc);
        _directionalLightPixelShader = Shader::Create(psDesc);

        // Spot light
        vsDesc = { .type = ShaderType::Vertex,
                   .debugName = "VS_SpotLight",
                   .filePath = "../Frost/resources/shaders/Light/VS_SpotLight.hlsl" };
        psDesc = { .type = ShaderType::Pixel,
                   .debugName = "PS_SpotLight",
                   .filePath = "../Frost/resources/shaders/Light/PS_SpotLight.hlsl" };
        _spotLightVertexShader = Shader::Create(vsDesc);
        _spotLightPixelShader = Shader::Create(psDesc);

        /*--------*/

        // Init light
        psDesc = { .type = ShaderType::Pixel,
                   .debugName = "PS_SpotLight",
                   .filePath = "../Frost/resources/shaders/Light/PS_InitLight.hlsl" };
        _initLightPixelShader = Shader::Create(psDesc);
        // Final light
        vsDesc = { .type = ShaderType::Vertex,
                   .debugName = "VS_SpotLight",
                   .filePath = "../Frost/resources/shaders/Light/VS_FinalLight.hlsl" };
        psDesc = { .type = ShaderType::Pixel,
                   .debugName = "PS_SpotLight",
                   .filePath = "../Frost/resources/shaders/Light/PS_FinalLight.hlsl" };
        _finalLightVertexShader = Shader::Create(vsDesc);
        _finalLightPixelShader = Shader::Create(psDesc);
    }

    void ShadowPipeline::Initialize()
    {
        constexpr size_t maxLightDataSize = std::max(
            { sizeof(DirectionalLightData), sizeof(PointLightData), sizeof(SpotLightData), sizeof(AmbiantLightData) });

        // Shaders
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .debugName = "VS_Shadow",
                              .filePath = "../Frost/resources/shaders/VS_Shadow.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .debugName = "PS_Shadow",
                              .filePath = "../Frost/resources/shaders/PS_Shadow.hlsl" };
        _shadowVertexShader = Shader::Create(vsDesc);
        _shadowPixelShader = Shader::Create(psDesc);

        InitLightShaders();

        // Constant Buffer
        auto* renderer = RendererAPI::GetRenderer();
        _vsShadowConstants = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER,
                                                                  .size = sizeof(VS_ShadowConstants),
                                                                  .dynamic = true,
                                                                  .debugName = "VS_ShadowConstants" });

        _lightPassBuffer = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER,
                                                                .size = maxLightDataSize,
                                                                .dynamic = true,
                                                                .debugName = "VS_PS_LightPassBuffer" });

        // Input Layout
        const uint32_t stride = sizeof(Math::Vector3);
        InputLayout::VertexAttributeArray attributes = { { .name = "POSITION",
                                                           .format = Format::RGB32_FLOAT,
                                                           .arraySize = 1,
                                                           .bufferIndex = 0,
                                                           .offset = 0,
                                                           .elementStride = stride,
                                                           .isInstanced = false } };
        _shadowInputLayout = std::make_unique<InputLayoutDX11>(attributes, *_shadowVertexShader);

        // ShadowSampler
        SamplerConfig shadowSamplerConfig = { .filter = Filter::MIN_MAG_MIP_LINEAR,
                                              .addressU = AddressMode::CLAMP,
                                              .addressV = AddressMode::CLAMP,
                                              .addressW = AddressMode::CLAMP,
                                              .comparisonFunction = ComparisonFunction::LESS_EQUAL };
        _shadowSampler = std::make_unique<SamplerDX11>(shadowSamplerConfig);

        // gBufferSampler
        SamplerConfig gBufferSamplerConfig = { .filter = Filter::MIN_MAG_MIP_POINT,
                                               .addressU = AddressMode::CLAMP,
                                               .addressV = AddressMode::CLAMP,
                                               .addressW = AddressMode::CLAMP };
        _gBufferSampler = std::make_unique<SamplerDX11>(gBufferSamplerConfig);
    }

    void ShadowPipeline::Shutdown()
    {
        // buffers
        _vsShadowConstants.reset();
        _lightPassBuffer.reset();

        // samplers
        _shadowSampler.reset();
        _gBufferSampler.reset();

        // shaders
        _shadowVertexShader.reset();
        _shadowPixelShader.reset();

        _pointLightVertexShader.reset();
        _pointLightPixelShader.reset();

        _directionalLightVertexShader.reset();
        _directionalLightPixelShader.reset();

        _ambiantLightVertexShader.reset();
        _ambiantLightPixelShader.reset();

        _spotLightVertexShader.reset();
        _spotLightPixelShader.reset();

        _initLightPixelShader.reset();

        _finalLightVertexShader.reset();
        _finalLightPixelShader.reset();

        // textures
        for_each(
            _shadowMaps.begin(), _shadowMaps.end(), [](auto& shadowPair) { shadowPair.second.shadowTexture.reset(); });
        _shadowMaps.clear();

        _albedoTexture.reset();
        _normalTexture.reset();
        _worldPositionTexture.reset();
        _materialTexture.reset();

        _luminanceTexture1.reset();
        _luminanceTexture2.reset();
        _finalLitTexture.reset();

        _shadowInputLayout.reset();
        _commandList.reset();
    }

    void ShadowPipeline::CreateTextures(uint32_t width, uint32_t height)
    {
        for_each(
            _shadowMaps.begin(), _shadowMaps.end(), [](auto& shadowPair) { shadowPair.second.shadowTexture.reset(); });

        _albedoTexture.reset();
        _normalTexture.reset();
        _worldPositionTexture.reset();
        _materialTexture.reset();

        _luminanceTexture1.reset();
        _luminanceTexture2.reset();
        _finalLitTexture.reset();

        TextureConfig lumConfig = { .format = Format::RGBA32_FLOAT,
                                    .width = width,
                                    .height = height,
                                    .isRenderTarget = true,
                                    .isShaderResource = true,
                                    .hasMipmaps = false };
        _luminanceTexture1 = std::make_shared<TextureDX11>(lumConfig);
        _luminanceTexture2 = std::make_shared<TextureDX11>(lumConfig);

        TextureConfig litConfig = { .format = Format::RGBA8_UNORM,
                                    .width = width,
                                    .height = height,
                                    .isRenderTarget = true,
                                    .isShaderResource = true,
                                    .hasMipmaps = false };
        _finalLitTexture = std::make_unique<TextureDX11>(litConfig);
    }

    void ShadowPipeline::OnResize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        if (_currentWidth != width || _currentHeight != height)
        {
            _currentWidth = width;
            _currentHeight = height;

            _vsShadowConstants.reset();
            _lightPassBuffer.reset();

            constexpr size_t maxLightDataSize = std::max({ sizeof(DirectionalLightData),
                                                           sizeof(PointLightData),
                                                           sizeof(SpotLightData),
                                                           sizeof(AmbiantLightData) });

            // Constant Buffer
            auto* renderer = RendererAPI::GetRenderer();
            _vsShadowConstants = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER,
                                                                      .size = sizeof(VS_ShadowConstants),
                                                                      .dynamic = true,
                                                                      .debugName = "VS_ShadowConstants" });

            _lightPassBuffer = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER,
                                                                    .size = maxLightDataSize,
                                                                    .dynamic = true,
                                                                    .debugName = "VS_PS_LightPassBuffer" });

            CreateTextures(width, height);
        }
    }

    void ShadowPipeline::SetGBufferData(DeferredRenderingPipeline* deferredPipeline, Scene* scene)
    {
        _albedoTexture = deferredPipeline->GetAlbedoTexture();
        _normalTexture = deferredPipeline->GetNormalTexture();
        _worldPositionTexture = deferredPipeline->GetWorldPositionTexture();
        _materialTexture = deferredPipeline->GetMaterialTexture();
        _commandList = deferredPipeline->GetSharedCommandList();
        _scene = scene;
    }

    void ShadowPipeline::MakePointDirectionalLight(Math::EulerAngles rot,
                                                   std::pair<Component::Light, Component::WorldTransform> lightPair)
    {
        // front
        std::pair<Component::Light, Component::WorldTransform> newlightPair = { Component::Light(),
                                                                                Component::WorldTransform() };
        newlightPair.first = lightPair.first;
        newlightPair.second = lightPair.second;
        newlightPair.second.SetRotation(rot);
        _virtualLightPairs.push_back(newlightPair);
    };

    void ShadowPipeline::MakeScaledDirectionalLight(std::pair<Component::Light, Component::WorldTransform> lightPair,
                                                    float cameraNear,
                                                    float cameraFar,
                                                    float cameraFOV,
                                                    const Component::WorldTransform& cameraTransform)
    {
        auto a = ComputeOrthoSize(cameraNear, cameraFar, cameraFOV, cameraTransform, lightPair.second);

        std::pair<Component::Light, Component::WorldTransform> newlightPair = { Component::Light(),
                                                                                Component::WorldTransform() };
        newlightPair.first = lightPair.first;
        newlightPair.second = lightPair.second;
        newlightPair.second.position = a.sunPos;

        auto* cfg = std::get_if<Component::LightDirectional>(&newlightPair.first.config);

        cfg->range = a.orthoSize;
        cfg->cascadeNear = cameraNear;
        cfg->cascadeFar = cameraFar;

        _virtualLightPairs.push_back(newlightPair);
    }

    void ShadowPipeline::ShadowPass(std::vector<std::pair<Component::Light, Component::WorldTransform>> lightPairs,
                                    const Component::Camera& camera,
                                    const Component::WorldTransform& cameraTransform,
                                    const Viewport& viewport)
    {
        _virtualLightPairs.clear();
        int i = 0;
        for (; i < lightPairs.size(); i++)
        {
            // Todo : si point light, générer 6 point lights directionelles
            if (lightPairs[i].first.GetType() == Component::LightType::Point)
            {
                MakePointDirectionalLight(Math::EulerAngles(0, 0, 0), lightPairs[i]);
                MakePointDirectionalLight(Math::EulerAngles(0.0_deg, 0.0_deg, 90.0_deg), lightPairs[i]);
                MakePointDirectionalLight(Math::EulerAngles(0.0_deg, 0.0_deg, -90.0_deg), lightPairs[i]);
                MakePointDirectionalLight(Math::EulerAngles(0.0_deg, 0.0_deg, 180.0_deg), lightPairs[i]);
                MakePointDirectionalLight(Math::EulerAngles(0.0_deg, 90.0_deg, 0.0_deg), lightPairs[i]);
                MakePointDirectionalLight(Math::EulerAngles(0.0_deg, -90.0_deg, 0.0_deg), lightPairs[i]);
            }
            else if (lightPairs[i].first.GetType() == Component::LightType::Directional)
            {
                float dist = camera.farClip - camera.nearClip;

                // Create 3 cascades for directional light
                MakeScaledDirectionalLight(lightPairs[i],
                                           camera.nearClip,
                                           camera.nearClip + dist * 0.1f,
                                           camera.perspectiveFOV.value(),
                                           cameraTransform);

                MakeScaledDirectionalLight(lightPairs[i],
                                           camera.nearClip + dist * 0.1f,
                                           camera.nearClip + dist * 0.5f,
                                           camera.perspectiveFOV.value(),
                                           cameraTransform);

                MakeScaledDirectionalLight(lightPairs[i],
                                           camera.nearClip + dist * 0.5f,
                                           camera.farClip,
                                           camera.perspectiveFOV.value(),
                                           cameraTransform);
            }
            else
                _virtualLightPairs.push_back(lightPairs[i]);
        }

        // Now compute shadow maps with UNIQUE IDs for each virtual light
        for (int j = 0; j < _virtualLightPairs.size(); j++)
        {
            // Use j as the unique ID for each virtual light
            if (_virtualLightPairs[j].first.GetType() == Component::LightType::Directional)
            {
                ComputeDirectionalShadowMap(LightObject{ j, _virtualLightPairs[j].second, _virtualLightPairs[j].first },
                                            cameraTransform,
                                            viewport);
            }
            else
                ComputeShadowMap(LightObject{ j, _virtualLightPairs[j].second, _virtualLightPairs[j].first },
                                 camera,
                                 cameraTransform,
                                 viewport);
        }
    }

    void ShadowPipeline::LightPass(const Component::Camera& camera,
                                   const Component::WorldTransform& cameraTransform,
                                   const Viewport& viewport)
    {
        InitLightTexture(viewport);

        for (int j = 0; j < _virtualLightPairs.size(); j++)
        {
            if (j % 2 != 0)
            {
                SubmitLight(camera,
                            cameraTransform,
                            LightObject{ j, _virtualLightPairs[j].second, _virtualLightPairs[j].first },
                            _luminanceTexture1,
                            _luminanceTexture2,
                            viewport);
            }
            else
            {
                SubmitLight(camera,
                            cameraTransform,
                            LightObject{ j, _virtualLightPairs[j].second, _virtualLightPairs[j].first },
                            _luminanceTexture2,
                            _luminanceTexture1,
                            viewport);
            }
        }
        if (_virtualLightPairs.size() % 2 == 0)
        {
            DrawFinalLitTexture(_luminanceTexture1, viewport);
        }
        else
        {
            DrawFinalLitTexture(_luminanceTexture2, viewport);
        }
    }

    void ShadowPipeline::InitLightTexture(const Viewport& viewport)
    {
        Texture* outPtr = _luminanceTexture1.get();
        _commandList->SetRenderTargets(1, &outPtr, nullptr);

        _commandList->SetShader(_finalLightVertexShader.get());
        _commandList->SetShader(_initLightPixelShader.get());

        _commandList->SetViewport(viewport.x * _currentWidth,
                                  viewport.y * _currentHeight,
                                  viewport.width * _currentWidth,
                                  viewport.height * _currentHeight,
                                  0.f,
                                  1.f);
        _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        _commandList->Draw(3, 0);
    }

    void ShadowPipeline::ComputeShadowMap(const LightObject& lightObj,
                                          const Component::Camera& camera,
                                          const Component::WorldTransform& cameraTransform,
                                          const Viewport& viewport)
    {
        const auto& light = lightObj.light;
        const auto& transform = lightObj.wt;

        auto it = _shadowMaps.find(lightObj.id);
        if (it == _shadowMaps.end())
        {
            ShadowData data;
            TextureConfig depthConfig = { .format = Format::R24G8_TYPELESS,
                                          .width = static_cast<uint32_t>(_shadowResolution),
                                          .height = static_cast<uint32_t>(_shadowResolution),
                                          .isRenderTarget = true,
                                          .isShaderResource = true,
                                          .hasMipmaps = false };
            data.shadowTexture = std::make_unique<TextureDX11>(depthConfig);
            it = _shadowMaps.emplace(lightObj.id, std::move(data)).first;
        }
        else if (it->second.shadowTexture == nullptr)
        {
            // Recreate texture if it was deleted
            TextureConfig depthConfig = { .format = Format::R24G8_TYPELESS,
                                          .width = static_cast<uint32_t>(_shadowResolution),
                                          .height = static_cast<uint32_t>(_shadowResolution),
                                          .isRenderTarget = true,
                                          .isShaderResource = true,
                                          .hasMipmaps = false };
            it->second.shadowTexture = std::make_unique<TextureDX11>(depthConfig);
        }

        ShadowData& shadowData = it->second;

        Vector3 up = Vector3(0, 1, 0);

        // Avoid colinearity
        if (fabs(transform.GetForward().y) > 0.99f)
            up = Vector3(1, 0, 0);

        auto lightView = Math::Matrix4x4::CreateLookToLH(transform.position, transform.GetForward(), up);
        Math::Matrix4x4 lightProj;

        if (light.GetType() == Component::LightType::Spot)
        {
            auto* cfg = std::get_if<Component::LightSpot>(&light.config);

            lightProj =
                Math::Matrix4x4::CreatePerspectiveFovLH(cfg->outerConeAngle.value() * 2.f, 1.f, 0.1f, cfg->range);
        }
        else if (light.GetType() == Component::LightType::Point)
        {
            auto* cfg = std::get_if<Component::LightPoint>(&light.config);

            lightProj = Math::Matrix4x4::CreatePerspectiveFovLH(DirectX::XM_PIDIV2, // champ de vision 90° par face
                                                                1.f,
                                                                0.1f,
                                                                cfg->radius);
        }

        shadowData.lightViewProj = lightView * lightProj;

        Texture* depthPtr = shadowData.shadowTexture.get();
        _commandList->SetRenderTargets(0, nullptr, depthPtr);
        _commandList->ClearDepthStencil(depthPtr, true, 1.0f, false, 0);

        _commandList->SetShader(_shadowVertexShader.get());
        _commandList->UnbindShader(ShaderType::Pixel);

        _commandList->SetViewport(0, 0, _shadowResolution, _shadowResolution, 0.f, 1.f);

        auto meshView = _scene->ViewActive<Component::StaticMesh, Component::WorldTransform>();
        meshView.each(
            [&](const Component::StaticMesh& staticMesh, const Component::WorldTransform& meshTransform)
            {
                if (staticMesh.GetModel())
                {
                    Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                    DrawDepthOnly(_commandList.get(), staticMesh, worldMatrix, shadowData.lightViewProj);
                }
            });
    }

    void ShadowPipeline::ComputeDirectionalShadowMap(const LightObject& lightObj,
                                                     const Component::WorldTransform& cameraTransform,
                                                     const Viewport& viewport)
    {
        const auto& light = lightObj.light;
        const auto& transform = lightObj.wt;

        auto it = _shadowMaps.find(lightObj.id);
        if (it == _shadowMaps.end())
        {
            ShadowData data;
            TextureConfig depthConfig = { .format = Format::R24G8_TYPELESS,
                                          .width = static_cast<uint32_t>(_shadowResolution),
                                          .height = static_cast<uint32_t>(_shadowResolution),
                                          .isRenderTarget = true,
                                          .isShaderResource = true,
                                          .hasMipmaps = false };
            data.shadowTexture = std::make_unique<TextureDX11>(depthConfig);
            it = _shadowMaps.emplace(lightObj.id, std::move(data)).first;
        }
        else if (it->second.shadowTexture == nullptr)
        {
            TextureConfig depthConfig = { .format = Format::R24G8_TYPELESS,
                                          .width = static_cast<uint32_t>(_shadowResolution),
                                          .height = static_cast<uint32_t>(_shadowResolution),
                                          .isRenderTarget = true,
                                          .isShaderResource = true,
                                          .hasMipmaps = false };
            it->second.shadowTexture = std::make_unique<TextureDX11>(depthConfig);
        }

        ShadowData& shadowData = it->second;
        auto* cfg = std::get_if<Component::LightDirectional>(&light.config);

        auto lightView =
            Math::Matrix4x4::CreateLookToLH(transform.position, transform.GetForward(), Math::Vector3(0, 1, 0));

        float orthoWidth = cfg->range;

        // Use large near/far range to capture all shadow casters
        // The cascade selection happens in the shader based on camera depth
        float orthoNear = 0.1f;
        float orthoFar = cfg->range * 10.0f; // Large enough to capture everything

        auto lightProj = Math::Matrix4x4::CreateOrthographicLH(orthoWidth, orthoWidth, orthoNear, orthoFar);

        shadowData.lightViewProj = lightView * lightProj;

        Texture* depthPtr = shadowData.shadowTexture.get();
        _commandList->SetRenderTargets(0, nullptr, depthPtr);
        _commandList->ClearDepthStencil(depthPtr, true, 1.0f, false, 0);

        _commandList->SetShader(_shadowVertexShader.get());
        _commandList->UnbindShader(ShaderType::Pixel);

        _commandList->SetViewport(0, 0, _shadowResolution, _shadowResolution, 0.0f, 1.0f);

        auto meshView = _scene->ViewActive<Component::StaticMesh, Component::WorldTransform>();
        meshView.each(
            [&](const Component::StaticMesh& staticMesh, const Component::WorldTransform& meshTransform)
            {
                if (staticMesh.GetModel())
                {
                    Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                    DrawDepthOnly(_commandList.get(), staticMesh, worldMatrix, shadowData.lightViewProj);
                }
            });
    }

    ShadowPipeline::DirectionalParams ShadowPipeline::ComputeOrthoSize(float cameraNear,
                                                                       float cameraFar,
                                                                       float cameraFOV,
                                                                       const Component::WorldTransform& cameraTransform,
                                                                       const Component::WorldTransform& sunTransform)
    {
        using namespace JPH;

        // Compute the 8 corners of the camera frustum slice
        float nearHeight = 2.0f * std::tan(cameraFOV / 2.0f) * cameraNear;
        float nearWidth = nearHeight;
        float farHeight = 2.0f * std::tan(cameraFOV / 2.0f) * cameraFar;
        float farWidth = farHeight;

        Vec3 cameraForward = vector_cast<Vec3>(cameraTransform.GetForward());
        Vec3 cameraRight = vector_cast<Vec3>(cameraTransform.GetRight());
        Vec3 cameraUp = vector_cast<Vec3>(cameraTransform.GetUp());
        Vec3 cameraPos = vector_cast<Vec3>(cameraTransform.position);

        // 8 frustum corners in world space
        Vec3 nearCenter = cameraPos + cameraForward * cameraNear;
        Vec3 farCenter = cameraPos + cameraForward * cameraFar;

        std::vector<Vec3> frustumCorners = { // Near plane
                                             nearCenter + cameraUp * (nearHeight / 2) - cameraRight * (nearWidth / 2),
                                             nearCenter + cameraUp * (nearHeight / 2) + cameraRight * (nearWidth / 2),
                                             nearCenter - cameraUp * (nearHeight / 2) - cameraRight * (nearWidth / 2),
                                             nearCenter - cameraUp * (nearHeight / 2) + cameraRight * (nearWidth / 2),
                                             // Far plane
                                             farCenter + cameraUp * (farHeight / 2) - cameraRight * (farWidth / 2),
                                             farCenter + cameraUp * (farHeight / 2) + cameraRight * (farWidth / 2),
                                             farCenter - cameraUp * (farHeight / 2) - cameraRight * (farWidth / 2),
                                             farCenter - cameraUp * (farHeight / 2) + cameraRight * (farWidth / 2)
        };

        // Create light space basis
        Vec3 lightForward = vector_cast<Vec3>(sunTransform.GetForward());
        Vec3 lightUp = Vec3(0, 1, 0);

        if (std::abs(lightForward.Dot(lightUp)) > 0.99f)
            lightUp = Vec3(1, 0, 0);

        Vec3 lightRight = lightUp.Cross(lightForward).Normalized();
        lightUp = lightForward.Cross(lightRight).Normalized();

        // Transform frustum corners to light space and find bounds
        float minX = FLT_MAX, maxX = -FLT_MAX;
        float minY = FLT_MAX, maxY = -FLT_MAX;
        float minZ = FLT_MAX, maxZ = -FLT_MAX;

        for (const auto& corner : frustumCorners)
        {
            Vec3 lightSpaceCorner;
            lightSpaceCorner.SetX(corner.Dot(lightRight));
            lightSpaceCorner.SetY(corner.Dot(lightUp));
            lightSpaceCorner.SetZ(corner.Dot(lightForward));

            minX = std::min(minX, lightSpaceCorner.GetX());
            maxX = std::max(maxX, lightSpaceCorner.GetX());
            minY = std::min(minY, lightSpaceCorner.GetY());
            maxY = std::max(maxY, lightSpaceCorner.GetY());
            minZ = std::min(minZ, lightSpaceCorner.GetZ());
            maxZ = std::max(maxZ, lightSpaceCorner.GetZ());
        }

        // Compute center in light space
        Vec3 centerLightSpace = Vec3((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f);

        // Transform back to world space
        Vec3 frustumCenter = lightRight * centerLightSpace.GetX() + lightUp * centerLightSpace.GetY() +
                             lightForward * centerLightSpace.GetZ();

        // Position the light BEHIND the frustum center along the light direction
        // This ensures consistent positioning regardless of camera orientation
        float depthRange = maxZ - minZ;
        float shadowDistance = depthRange * 2.0f; // Distance to place light behind frustum

        Vec3 sunPos = frustumCenter - lightForward * shadowDistance;

        // Compute ortho size
        float orthoWidth = std::max(maxX - minX, maxY - minY);
        orthoWidth *= 1.2f; // Add padding

        DirectionalParams d;
        d.orthoSize = orthoWidth;
        d.sunPos = vector_cast<Vector3>(sunPos);

        return d;
    }

    void ShadowPipeline::DrawDepthOnly(CommandList* cmd,
                                       const Component::StaticMesh& staticMesh,
                                       const Math::Matrix4x4& worldMatrix,
                                       const Math::Matrix4x4& _currentLightViewProj)
    {
        VS_ShadowConstants vsData;
        vsData.World = LoadMatrix(Math::Matrix4x4::CreateTranspose(worldMatrix));
        vsData.LightViewProj = LoadMatrix(Math::Matrix4x4::CreateTranspose(_currentLightViewProj));
        _vsShadowConstants->UpdateData(cmd, &vsData, sizeof(vsData));

        cmd->SetConstantBuffer(_vsShadowConstants.get(), 0);

        cmd->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        cmd->SetShader(_shadowVertexShader.get());
        cmd->UnbindShader(ShaderType::Pixel);
        cmd->SetInputLayout(_shadowInputLayout.get());
        cmd->SetRasterizerState(RasterizerMode::SolidCullBack);

        for (const auto& mesh : staticMesh.GetModel()->GetMeshes())
        {
            // Buffers
            cmd->SetVertexBuffer(mesh.GetVertexBuffer(), mesh.GetVertexStride(), 0);
            cmd->SetIndexBuffer(mesh.GetIndexBuffer(), 0);

            // Draw
            cmd->DrawIndexed(mesh.GetIndexCount(), 0, 0);
        }
    }

    void ShadowPipeline::SubmitLight(const Component::Camera& camera,
                                     const Component::WorldTransform& cameraTransform,
                                     LightObject lightObj,
                                     std::shared_ptr<Texture> destination,
                                     std::shared_ptr<Texture> source,
                                     const Viewport& viewport)
    {

        if (viewport.width == 0 || viewport.height == 0)
        {
            FT_ENGINE_ERROR("Viewport width and height should be > 0");
            return;
        }
        if (!_albedoTexture)
            return;

        auto it = _shadowMaps.find(lightObj.id);
        if (it == _shadowMaps.end())
            return;

        ShadowData& shadow = it->second;

        switch (lightObj.light.GetType())
        {
            case Component::LightType::Directional:
            {
                auto* cfg = std::get_if<Component::LightDirectional>(&lightObj.light.config);

                auto light = DirectionalLightData();
                light.shadowResolution = _shadowResolution;
                light.CameraPosition = cameraTransform.position;
                light.Direction = lightObj.wt.GetForward();
                light.Color = lightObj.light.color;
                light.Intensity = lightObj.light.intensity;
                light.CascadeNear = cfg->cascadeNear;
                light.CameraFwd = cameraTransform.GetForward();
                light.CascadeFar = cfg->cascadeFar;
                light.LightPosition = lightObj.wt.position;

                _lightPassBuffer->UpdateData(_commandList.get(), &light, sizeof(light));
                DrawLight(
                    shadow, _directionalLightVertexShader, _directionalLightPixelShader, destination, source, viewport);
                break;
            }
            case Component::LightType::Point:
            {
                auto* cfg = std::get_if<Component::LightPoint>(&lightObj.light.config);
                // pointData.Falloff = cfg->falloff;

                auto light = PointLightData();
                light.CameraPosition = cameraTransform.position;
                light.Position = lightObj.wt.position;
                light.Radius = cfg->radius;
                light.Color = lightObj.light.color;
                light.Intensity = lightObj.light.intensity;
                light.ShadowResolution = _shadowResolution;
                light.LightDirection = lightObj.wt.GetForward();

                _lightPassBuffer->UpdateData(_commandList.get(), &light, sizeof(light));
                DrawLight(shadow, _pointLightVertexShader, _pointLightPixelShader, destination, source, viewport);
                break;
            }
            case Component::LightType::Spot:
            {
                auto* cfg = std::get_if<Component::LightSpot>(&lightObj.light.config);

                auto light = SpotLightData();
                light.CameraPosition = cameraTransform.position;
                light.Position = lightObj.wt.position;
                light.Direction = lightObj.wt.GetForward();
                light.Color = lightObj.light.color;
                light.Intensity = lightObj.light.intensity;
                light.shadowResolution = _shadowResolution;

                light.InnerConeAngle = std::cos(cfg->innerConeAngle.value());
                light.OuterConeAngle = std::cos(cfg->outerConeAngle.value());
                light.Radius = cfg->range;

                _lightPassBuffer->UpdateData(_commandList.get(), &light, sizeof(light));
                DrawLight(shadow, _spotLightVertexShader, _spotLightPixelShader, destination, source, viewport);
                break;
            }
            case Component::LightType::Ambiant:
            {
                auto light = AmbiantLightData();
                light.CameraPosition = cameraTransform.position;
                light.Color = lightObj.light.color;
                light.Intensity = lightObj.light.intensity;

                _lightPassBuffer->UpdateData(_commandList.get(), &light, sizeof(light));
                DrawLight(shadow, _ambiantLightVertexShader, _ambiantLightPixelShader, destination, source, viewport);
                break;
            }
        }
    }

    void ShadowPipeline::DrawLight(ShadowData& shadow,
                                   std::shared_ptr<Shader> lightingVertexShader,
                                   std::shared_ptr<Shader> lightingPixelShader,
                                   std::shared_ptr<Texture> destination,
                                   std::shared_ptr<Texture> source,
                                   const Viewport& viewport)
    {
        Texture* outPtr = destination.get();
        _commandList->SetRenderTargets(1, &outPtr, nullptr);

        // UPDATE: Send the correct light's view-projection matrix to the shader
        VS_ShadowConstants vsData;
        vsData.World = DirectX::XMMatrixIdentity(); // Not used in light pass
        vsData.LightViewProj = LoadMatrix(Math::Matrix4x4::CreateTranspose(shadow.lightViewProj));
        _vsShadowConstants->UpdateData(_commandList.get(), &vsData, sizeof(vsData));

        _commandList->SetConstantBuffer(_lightPassBuffer.get(), 0);
        _commandList->SetConstantBuffer(_vsShadowConstants.get(), 1);

        _commandList->SetTexture(_albedoTexture.get(), 0);
        _commandList->SetTexture(_normalTexture.get(), 1);
        _commandList->SetTexture(_worldPositionTexture.get(), 2);
        _commandList->SetTexture(_materialTexture.get(), 3);

        _commandList->SetTexture(shadow.shadowTexture.get(), 4);
        _commandList->SetTexture(source.get(), 5);
        _commandList->SetSampler(_gBufferSampler.get(), 0);

        _commandList->SetInputLayout(nullptr);
        _commandList->SetShader(lightingVertexShader.get());
        _commandList->SetShader(lightingPixelShader.get());

        _commandList->SetViewport(viewport.x * _currentWidth,
                                  viewport.y * _currentHeight,
                                  viewport.width * _currentWidth,
                                  viewport.height * _currentHeight,
                                  0.f,
                                  1.f);
        _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        _commandList->UnbindShader(ShaderType::Geometry);
        _commandList->UnbindShader(ShaderType::Hull);
        _commandList->UnbindShader(ShaderType::Domain);
        _commandList->Draw(3, 0);
    }

    void ShadowPipeline::DrawFinalLitTexture(std::shared_ptr<Texture> luminanceTexture, const Viewport& viewport)
    {
        Texture* outPtr = _finalLitTexture.get();
        _commandList->SetRenderTargets(1, &outPtr, nullptr);

        _commandList->SetTexture(_albedoTexture.get(), 0);
        _commandList->SetTexture(luminanceTexture.get(), 1);
        _commandList->SetSampler(_gBufferSampler.get(), 0);

        _commandList->SetShader(_finalLightVertexShader.get());
        _commandList->SetShader(_finalLightPixelShader.get());

        _commandList->SetViewport(viewport.x * _currentWidth,
                                  viewport.y * _currentHeight,
                                  viewport.width * _currentWidth,
                                  viewport.height * _currentHeight,
                                  0.f,
                                  1.f);
        _commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        _commandList->UnbindShader(ShaderType::Geometry);
        _commandList->UnbindShader(ShaderType::Hull);
        _commandList->UnbindShader(ShaderType::Domain);
        _commandList->Draw(3, 0);
    }

} // namespace Frost