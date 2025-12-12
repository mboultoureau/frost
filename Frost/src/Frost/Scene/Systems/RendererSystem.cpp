#include "Frost/Scene/Systems/RendererSystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Debugging/DebugInterface/DebugRendering.h"
#include "Frost/Debugging/DebugInterface/DebugPhysics.h"
#include "Frost/Physics/Physics.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Scene/Components/RelativeView.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Renderer/Pipeline/JoltDebugRenderingPipeline.h"
#include "Frost/Renderer/Frustum.h"

using namespace Frost::Component;

namespace Frost
{
    RendererSystem::RendererSystem() : _frustum{} {}

    void RendererSystem::LateUpdate(Scene& scene, float deltaTime)
    {
        float currentWidth = _externalRenderTarget ? (float)_externalRenderTarget->GetWidth()
                                                   : (float)Application::GetWindow()->GetWidth();
        float currentHeight = _externalRenderTarget ? (float)_externalRenderTarget->GetHeight()
                                                    : (float)Application::GetWindow()->GetHeight();

        if (currentWidth == 0 || currentHeight == 0)
        {
            return;
        }

        auto cameraView = scene.ViewActive<Camera, WorldTransform>();
        auto lightView = scene.ViewActive<Light, WorldTransform>();
        auto meshView = scene.ViewActive<StaticMesh, WorldTransform>();

        std::vector<std::pair<Component::Light, Component::WorldTransform>> allLights;
        allLights.reserve(lightView.size_hint());
        lightView.each([&](const Component::Light& light, const Component::WorldTransform& transform)
                       { allLights.emplace_back(light, transform); });

        std::vector<RenderCameraData> renderTargetCameras;
        std::vector<RenderCameraData> mainCameras;

        cameraView.each(
            [&](entt::entity entity, const Camera& camera, const WorldTransform& transform)
            {
                if (camera.renderTargetConfig.has_value())
                {
                    renderTargetCameras.push_back({ entity, &camera, &transform });
                }
                else
                {
                    mainCameras.push_back({ entity, &camera, &transform });
                }
            });

        auto sortCam = [](const RenderCameraData& a, const RenderCameraData& b)
        { return a.camera->priority < b.camera->priority; };
        std::sort(renderTargetCameras.begin(), renderTargetCameras.end(), sortCam);
        std::sort(mainCameras.begin(), mainCameras.end(), sortCam);

        const float mainAspectRatio = (currentHeight > 0) ? (currentWidth / currentHeight) : 1.0f;

        for (const auto& camData : renderTargetCameras)
        {
            const auto& config = camData.camera->renderTargetConfig.value();

            std::shared_ptr<Texture> renderTarget;
            auto it = _renderTargetCache.find(camData.entity);
            if (it != _renderTargetCache.end() && it->second->GetWidth() == config.width &&
                it->second->GetHeight() == config.height)
            {
                renderTarget = it->second;
            }
            else
            {
                TextureConfig texConfig = { .debugName = "CameraTarget",
                                            .format = Format::RGBA8_UNORM,
                                            .width = config.width,
                                            .height = config.height,
                                            .isRenderTarget = true,
                                            .isShaderResource = true };
                renderTarget = Texture::Create(texConfig);
                _renderTargetCache[camData.entity] = renderTarget;
            }

            float aspectRatioToUse = config.useScreenSpaceAspectRatio ? mainAspectRatio : 0.0f;
            _RenderSceneToTexture(scene, camData, deltaTime, allLights, renderTarget, aspectRatioToUse);
        }

        meshView.each(
            [&](StaticMesh& staticMesh, const WorldTransform&)
            {
                if (staticMesh.GetModel())
                {
                    for (auto& material : staticMesh.GetModel()->GetMaterials())
                    {
                        if (material.cameraRef != entt::null)
                        {
                            auto it = _renderTargetCache.find(static_cast<entt::entity>(material.cameraRef));
                            if (it != _renderTargetCache.end())
                            {
                                material.albedoTextures.clear();
                                material.albedoTextures.push_back(it->second);
                            }
                        }
                    }
                }
            });

        JoltRenderingPipeline* joltDebugRenderer = static_cast<JoltRenderingPipeline*>(Physics::GetDebugRenderer());

        for (const auto& camData : mainCameras)
        {
            if (!Debug::RendererConfig::display && !Debug::PhysicsConfig::IsDisplayEnabled())
            {
                continue;
            }

            const Camera& camera = *camData.camera;
            const WorldTransform& cameraTransform = *camData.transform;

            Texture* finalRenderTarget =
                _externalRenderTarget ? _externalRenderTarget.get() : RendererAPI::GetRenderer()->GetBackBuffer();
            Viewport mainRenderViewport = { camera.viewport.x * currentWidth,
                                            camera.viewport.y * currentHeight,
                                            camera.viewport.width * currentWidth,
                                            camera.viewport.height * currentHeight };
            if (mainRenderViewport.width == 0 || mainRenderViewport.height == 0)
                continue;

            const float mainCameraAspectRatio =
                (mainRenderViewport.height > 0) ? (mainRenderViewport.width / mainRenderViewport.height) : 1.0f;
            _deferredRendering.OnResize(static_cast<uint32_t>(mainRenderViewport.width),
                                        static_cast<uint32_t>(mainRenderViewport.height));
            _shadowPipeline.OnResize(static_cast<uint32_t>(mainRenderViewport.width),
                                     static_cast<uint32_t>(mainRenderViewport.height));

            Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(cameraTransform);
            Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(camera, mainCameraAspectRatio);
            Math::Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

            if (camera.frustumCulling)
            {
                DirectX::XMMATRIX viewProj = Math::LoadMatrix(viewMatrix) * Math::LoadMatrix(projectionMatrix);
                _frustum.Extract(viewProj, camera.frustumPadding);
            }

            // Light Culling
            std::vector<std::pair<Component::Light, Component::WorldTransform>> visibleLights;
            visibleLights.reserve(lightView.size_hint());

            lightView.each(
                [&](const Component::Light& light, const Component::WorldTransform& transform)
                {
                    bool isVisible = false;
                    switch (light.GetType())
                    {
                        case Component::LightType::Directional:
                        case Component::LightType::Ambiant:
                            isVisible = true;
                            break;
                        case Component::LightType::Point:
                        {
                            auto* cfg = std::get_if<Component::LightPoint>(&light.config);
                            BoundingBox lightBox;
                            float r = cfg->radius;
                            lightBox.min = { transform.position.x - r,
                                             transform.position.y - r,
                                             transform.position.z - r };
                            lightBox.max = { transform.position.x + r,
                                             transform.position.y + r,
                                             transform.position.z + r };

                            if (!camera.frustumCulling || _frustum.IsInside(lightBox))
                                isVisible = true;
                            break;
                        }
                        case Component::LightType::Spot:
                        {
                            auto* cfg = std::get_if<Component::LightSpot>(&light.config);
                            BoundingBox lightBox;
                            float r = cfg->range;
                            lightBox.min = { transform.position.x - r,
                                             transform.position.y - r,
                                             transform.position.z - r };
                            lightBox.max = { transform.position.x + r,
                                             transform.position.y + r,
                                             transform.position.z + r };

                            if (!camera.frustumCulling || _frustum.IsInside(lightBox))
                                isVisible = true;
                            break;
                        }
                    }

                    if (isVisible)
                    {
                        visibleLights.emplace_back(light, transform);
                    }
                });

            CommandList* commandList = _deferredRendering.GetCommandList();
            commandList->BeginRecording();

            std::shared_ptr<Texture> skyboxTexture = nullptr;
            if (scene.GetRegistry().all_of<Skybox>(camData.entity))
            {
                const auto& skyboxComponent = scene.GetRegistry().get<Skybox>(camData.entity);
                skyboxTexture = _GetOrCreateSkyboxTexture(skyboxComponent);
            }

            if (Debug::RendererConfig::display)
            {
                // Apply post-processing effects pre-render
                for (auto& effect : camera.postEffects)
                {
                    if (!effect->IsEnabled())
                        continue;
                    FT_ENGINE_ASSERT(effect != nullptr, "PostEffect is null");
                    effect->OnPreRender(deltaTime, viewMatrix, projectionMatrix);
                }

                _deferredRendering.BeginFrame(
                    camera, cameraTransform, viewMatrix, projectionMatrix, mainRenderViewport);

                meshView.each(
                    [&](const StaticMesh& staticMesh, const WorldTransform& meshTransform)
                    {
                        if (staticMesh.GetModel())
                        {
                            Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                            if (!camera.frustumCulling || _IsVisible(staticMesh, worldMatrix))
                            {
                                _deferredRendering.SubmitModel(*staticMesh.GetModel(), worldMatrix);
                            }
                        }
                    });

                _shadowPipeline.SetGBufferData(&_deferredRendering, &scene);

                _shadowPipeline.ShadowPass(visibleLights, camera, cameraTransform, camera.viewport);
                _shadowPipeline.LightPass(camera, cameraTransform, camera.viewport);

                if (skyboxTexture)
                {
                    _skyboxPipeline.Render(commandList,
                                           _shadowPipeline.GetFinalLitTexture(),
                                           _deferredRendering.GetDepthStencilTexture().get(),
                                           skyboxTexture.get(),
                                           camera,
                                           cameraTransform);
                }
            }
            else
            {
                _deferredRendering.BeginFrame(
                    camera, cameraTransform, viewMatrix, projectionMatrix, mainRenderViewport);
                _shadowPipeline.SetGBufferData(&_deferredRendering, &scene);
                _shadowPipeline.ShadowPass(visibleLights, camera, cameraTransform, camera.viewport);
                _shadowPipeline.LightPass(camera, cameraTransform, camera.viewport);
            }

            Texture* sceneTexture = _shadowPipeline.GetFinalLitTexture();
            _ApplyPostProcessing(commandList, sceneTexture, finalRenderTarget, camera, deltaTime);

#ifdef FT_DEBUG
            if (Debug::PhysicsConfig::IsDisplayEnabled() && joltDebugRenderer)
            {
                joltDebugRenderer->Render(commandList,
                                          mainRenderViewport,
                                          viewProjectionMatrix,
                                          camera,
                                          finalRenderTarget,
                                          _deferredRendering.GetDepthStencilTexture().get());
            }
#endif

            commandList->EndRecording();
            commandList->Execute();
        }

        if (joltDebugRenderer)
        {
            joltDebugRenderer->Clear();
        }

        RendererAPI::GetRenderer()->RestoreBackBufferRenderTarget();
    }

    bool RendererSystem::_IsVisible(const Component::StaticMesh& staticMesh, const Math::Matrix4x4& worldMatrix)
    {
        bool renderModel = false;
        DirectX::XMMATRIX matWorld = Math::LoadMatrix(worldMatrix);

        for (auto& mesh : staticMesh.GetModel()->GetMeshes())
        {
            BoundingBox worldBox = BoundingBox::TransformAABB(mesh.GetBoundingBox(), matWorld);
            bool renderMesh = _frustum.IsInside(worldBox);
            if (renderMesh)
            {
                renderModel = true;
            }

            mesh.enabled = renderMesh;
        }
        return renderModel;
    }

    void RendererSystem::_ApplyPostProcessing(CommandList* commandList,
                                              Texture* sourceTexture,
                                              Texture* destinationTarget,
                                              const Camera& camera,
                                              float deltaTime)
    {
        std::vector<std::shared_ptr<PostEffect>> postProcessingPasses;
        for (const auto& effect : camera.postEffects)
        {
            if (effect && effect->IsEnabled() && effect->IsPostProcessingPass())
            {
                postProcessingPasses.push_back(effect);
            }
        }

        if (!postProcessingPasses.empty())
        {
            uint32_t width = sourceTexture->GetWidth();
            uint32_t height = sourceTexture->GetHeight();
            if (!_source || _source->GetWidth() != width || _source->GetHeight() != height)
            {
                TextureConfig ppConfig = { .format = sourceTexture->GetFormat(),
                                           .width = width,
                                           .height = height,
                                           .isRenderTarget = true,
                                           .isShaderResource = true };
                _source = Texture::Create(ppConfig);
                _destination = Texture::Create(ppConfig);
            }

            Texture* source = sourceTexture;
            Texture* destination = nullptr;

            for (size_t i = 0; i < postProcessingPasses.size(); ++i)
            {
                destination = (i == postProcessingPasses.size() - 1)
                                  ? destinationTarget
                                  : ((i % 2 == 0) ? _destination.get() : _source.get());
                postProcessingPasses[i]->SetNormalTexture(_deferredRendering.GetNormalTexture().get());
                postProcessingPasses[i]->SetMaterialTexture(_deferredRendering.GetMaterialTexture().get());
                postProcessingPasses[i]->SetDepthTexture(_deferredRendering.GetDepthStencilTexture().get());
                postProcessingPasses[i]->OnPostRender(deltaTime, commandList, source, destination);
                source = destination;
            }
        }
        else
        {
            if (destinationTarget->GetWidth() == sourceTexture->GetWidth() &&
                destinationTarget->GetHeight() == sourceTexture->GetHeight())
            {
                commandList->CopyResource(destinationTarget, sourceTexture);
            }
        }
    }

    void RendererSystem::_RenderSceneToTexture(
        Scene& scene,
        const RenderCameraData& camData,
        float deltaTime,
        const std::vector<std::pair<Component::Light, Component::WorldTransform>>& allLights,
        const std::shared_ptr<Texture>& renderTarget,
        float overrideAspectRatio)
    {
        const Camera& camera = *camData.camera;
        const WorldTransform& cameraTransform = *camData.transform;

        if (!renderTarget)
            return;

        auto meshView = scene.ViewActive<StaticMesh, WorldTransform>();

        float targetWidth = static_cast<float>(renderTarget->GetWidth());
        float targetHeight = static_cast<float>(renderTarget->GetHeight());
        if (targetWidth == 0 || targetHeight == 0)
            return;

        Viewport renderViewport = { 0.0f, 0.0f, targetWidth, targetHeight };

        _deferredRendering.OnResize(static_cast<uint32_t>(targetWidth), static_cast<uint32_t>(targetHeight));
        _shadowPipeline.OnResize(static_cast<uint32_t>(targetWidth), static_cast<uint32_t>(targetHeight));

        float aspectRatio = (renderViewport.height > 0) ? (renderViewport.width / renderViewport.height) : 1.0f;
        if (overrideAspectRatio > 0.0f)
        {
            aspectRatio = overrideAspectRatio;
        }

        Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(cameraTransform);
        Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(camera, aspectRatio);

        // --- CULLING : Calcul du Frustum LOCAL ---
        Frustum localFrustum;
        if (camera.frustumCulling)
        {
            DirectX::XMMATRIX viewProj = Math::LoadMatrix(viewMatrix) * Math::LoadMatrix(projectionMatrix);
            localFrustum.Extract(viewProj, camera.frustumPadding);
        }

        // --- CULLING : Filtrage des lumières pour cette caméra ---
        std::vector<std::pair<Component::Light, Component::WorldTransform>> visibleLights;
        auto lightView = scene.ViewActive<Light, WorldTransform>();

        visibleLights.reserve(lightView.size_hint());
        lightView.each(
            [&](const Component::Light& light, const Component::WorldTransform& transform)
            {
                bool isVisible = false;
                switch (light.GetType())
                {
                    case Component::LightType::Directional:
                    case Component::LightType::Ambiant:
                        isVisible = true;
                        break;
                    case Component::LightType::Point:
                    {
                        auto* cfg = std::get_if<Component::LightPoint>(&light.config);
                        BoundingBox lightBox;
                        float r = cfg->radius;
                        lightBox.min = { transform.position.x - r, transform.position.y - r, transform.position.z - r };
                        lightBox.max = { transform.position.x + r, transform.position.y + r, transform.position.z + r };

                        if (!camera.frustumCulling || localFrustum.IsInside(lightBox))
                            isVisible = true;
                        break;
                    }
                    case Component::LightType::Spot:
                    {
                        auto* cfg = std::get_if<Component::LightSpot>(&light.config);
                        BoundingBox lightBox;
                        float r = cfg->range;
                        lightBox.min = { transform.position.x - r, transform.position.y - r, transform.position.z - r };
                        lightBox.max = { transform.position.x + r, transform.position.y + r, transform.position.z + r };

                        if (!camera.frustumCulling || localFrustum.IsInside(lightBox))
                            isVisible = true;
                        break;
                    }
                }
                if (isVisible)
                    visibleLights.emplace_back(light, transform);
            });

        CommandList* commandList = _deferredRendering.GetCommandList();
        commandList->BeginRecording();

        _deferredRendering.BeginFrame(camera, cameraTransform, viewMatrix, projectionMatrix, renderViewport);

        meshView.each(
            [&](const StaticMesh& staticMesh, const WorldTransform& meshTransform)
            {
                if (staticMesh.GetModel())
                {
                    Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                    _deferredRendering.SubmitModel(*staticMesh.GetModel(), worldMatrix);
                }
            });

        _shadowPipeline.SetGBufferData(&_deferredRendering, &scene);

        _shadowPipeline.ShadowPass(visibleLights, camera, cameraTransform, camera.viewport);
        _shadowPipeline.LightPass(camera, cameraTransform, camera.viewport);

        std::shared_ptr<Texture> skyboxTexture = nullptr;
        if (scene.GetRegistry().all_of<Skybox>(camData.entity))
        {
            const auto& skyboxComponent = scene.GetRegistry().get<Skybox>(camData.entity);
            skyboxTexture = _GetOrCreateSkyboxTexture(skyboxComponent);
        }

        if (skyboxTexture)
        {
            _skyboxPipeline.Render(commandList,
                                   renderTarget.get(),
                                   _deferredRendering.GetDepthStencilTexture().get(),
                                   skyboxTexture.get(),
                                   camera,
                                   cameraTransform);
        }

        commandList->EndRecording();
        commandList->Execute();
    }

    std::shared_ptr<Texture> RendererSystem::_GetOrCreateSkyboxTexture(const Component::Skybox& skybox)
    {
        std::string cacheKey;
        TextureConfig textureConfig;
        textureConfig.layout = TextureLayout::CUBEMAP;
        textureConfig.hasMipmaps = true;

        bool isValid = false;

        std::visit(
            [&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, SkyboxSourceCubemap>)
                {
                    if (!arg.filepath.empty())
                    {
                        cacheKey = arg.filepath.generic_string();
                        textureConfig.path = cacheKey;
                        textureConfig.isUnfoldedCubemap = true;
                        isValid = true;
                    }
                }
                else if constexpr (std::is_same_v<T, SkyboxSource6Files>)
                {
                    std::stringstream ss;
                    bool allFacesSet = true;
                    for (int i = 0; i < 6; ++i)
                    {
                        if (arg.faceFilepaths[i].empty())
                        {
                            allFacesSet = false;
                            textureConfig.isUnfoldedCubemap = false;
                            break;
                        }
                        ss << arg.faceFilepaths[i].generic_string() << ";";
                        textureConfig.faceFilePaths[i] = arg.faceFilepaths[i].generic_string();
                    }

                    if (allFacesSet)
                    {
                        cacheKey = ss.str();
                        isValid = true;
                    }
                }
            },
            skybox.config);

        if (!isValid)
        {
            return nullptr;
        }

        auto it = _skyboxTextureCache.find(cacheKey);
        if (it != _skyboxTextureCache.end())
        {
            return it->second;
        }

        FT_ENGINE_INFO("Creating new skybox texture from source: {0}", cacheKey);
        std::shared_ptr<Texture> newTexture = Texture::Create(textureConfig);
        _skyboxTextureCache[cacheKey] = newTexture;
        return newTexture;
    }
} // namespace Frost