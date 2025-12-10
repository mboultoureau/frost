#include "Frost/Scene/Systems/RendererSystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Debugging/DebugInterface/DebugRendering.h"
#include "Frost/Debugging/DebugInterface/DebugPhysics.h"
#include "Frost/Physics/Physics.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Scene/Components/RelativeView.h"
#include "Frost/Scene/Components/Skybox.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Renderer/Pipeline/JoltDebugRenderingPipeline.h"

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
        auto virtualCameraView = scene.ViewActive<VirtualCamera, WorldTransform>();
        auto lightView = scene.ViewActive<Light, WorldTransform>();
        auto skyboxView = scene.ViewActive<Skybox>();
        auto meshView = scene.ViewActive<StaticMesh, WorldTransform>();

        std::vector<std::pair<Component::Light, Component::WorldTransform>> allLights;
        allLights.reserve(lightView.size_hint());
        lightView.each([&](const Component::Light& light, const Component::WorldTransform& transform)
                       { allLights.emplace_back(light, transform); });

        std::shared_ptr<Texture> skyboxTexture{};
        skyboxView.each(
            [&](const Skybox& skybox)
            {
                if (!skyboxTexture)
                {
                    skyboxTexture = skybox.cubemapTexture;
                }
            });

        std::vector<RenderCameraData> virtualCameras;
        std::vector<RenderCameraData> classicCameras;
        virtualCameras.reserve(virtualCameraView.size_hint());
        classicCameras.reserve(cameraView.size_hint());

        cameraView.each([&](entt::entity entity, const Camera& camera, const WorldTransform& transform)
                        { classicCameras.push_back({ entity, &camera, &transform }); });

        virtualCameraView.each(
            [&](entt::entity entity, const VirtualCamera& virtualCamera, const WorldTransform& transform)
            { virtualCameras.push_back({ entity, &virtualCamera, &transform }); });

        auto sortCam = [](const RenderCameraData& a, const RenderCameraData& b)
        { return a.camera->priority < b.camera->priority; };
        std::sort(virtualCameras.begin(), virtualCameras.end(), sortCam);
        std::sort(classicCameras.begin(), classicCameras.end(), sortCam);

        JoltRenderingPipeline* joltDebugRenderer = static_cast<JoltRenderingPipeline*>(Physics::GetDebugRenderer());

        for (const auto& camData : classicCameras)
        {
            if (!Debug::RendererConfig::display && !Debug::PhysicsConfig::display)
            {
                continue;
            }

            std::unordered_map<entt::entity, std::shared_ptr<Texture>> virtualCameraTargets;
            Viewport mainRenderViewport = { camData.camera->viewport.x * currentWidth,
                                            camData.camera->viewport.y * currentHeight,
                                            camData.camera->viewport.width * currentWidth,
                                            camData.camera->viewport.height * currentHeight };
            const float mainCameraAspectRatio =
                (mainRenderViewport.height > 0) ? (mainRenderViewport.width / mainRenderViewport.height) : 1.0f;

            for (const auto& virtualCamData : virtualCameras)
            {
                const VirtualCamera& virtualCam = static_cast<const VirtualCamera&>(*virtualCamData.camera);

                float aspectRatioToUse = 0.0f;
                if (virtualCam.useScreenSpaceAspectRatio)
                {
                    aspectRatioToUse = mainCameraAspectRatio;
                }

                WorldTransform originalVirtualCamTransform = *virtualCamData.transform;

                if (scene.GetRegistry().all_of<RelativeView>(virtualCamData.entity))
                {
                    const auto& relativeView = scene.GetRegistry().get<RelativeView>(virtualCamData.entity);
                    if (scene.GetRegistry().valid(relativeView.referenceEntity))
                    {
                        /*const auto& referenceTransform =
                            scene.GetRegistry().get<WorldTransform>(relativeView.referenceEntity);

                        Math::Matrix4x4 matPlayer = Math::GetTransformMatrix(*camData.transform);
                        Math::Matrix4x4 matRef = Math::GetTransformMatrix(referenceTransform);
                        Math::Matrix4x4 matOut = Math::GetTransformMatrix(*virtualCamData.transform);

                        Math::Matrix4x4 matRel = matPlayer * Math::Matrix4x4::Invert(matRef);
                        Math::Matrix4x4 matNew = matRel * relativeView.modifier * matOut;

                        // Modifier temporairement la transformation de la caméra virtuelle
                        auto& camTransform = scene.GetRegistry().get<WorldTransform>(virtualCamData.entity);
                        Math::DecomposeTransform(
                            matNew, camTransform.position, camTransform.rotation, camTransform.scale);*/
                    }
                }

                _RenderSceneToTexture(scene, virtualCamData, deltaTime, allLights, skyboxTexture, aspectRatioToUse);
                virtualCameraTargets[virtualCamData.entity] = virtualCam.GetRenderTarget();

                if (scene.GetRegistry().all_of<RelativeView>(virtualCamData.entity))
                {
                    // scene.GetRegistry().get<WorldTransform>(virtualCamData.entity) = originalVirtualCamTransform;
                }
            }

            meshView.each(
                [&](StaticMesh& staticMesh, const WorldTransform& /*unused*/)
                {
                    if (staticMesh.GetModel())
                    {
                        for (auto& material : staticMesh.GetModel()->GetMaterials())
                        {
                            if (material.cameraRef != entt::null)
                            {
                                auto it = virtualCameraTargets.find(static_cast<entt::entity>(material.cameraRef));
                                if (it != virtualCameraTargets.end())
                                {
                                    material.albedoTextures.clear();
                                    material.albedoTextures.push_back(it->second);
                                }
                            }
                        }
                    }
                });

            const Camera& camera = *camData.camera;
            const WorldTransform& cameraTransform = *camData.transform;

            Texture* finalRenderTarget =
                _externalRenderTarget ? _externalRenderTarget.get() : RendererAPI::GetRenderer()->GetBackBuffer();
            if (mainRenderViewport.width == 0 || mainRenderViewport.height == 0)
                continue;

            _deferredRendering.OnResize(static_cast<uint32_t>(mainRenderViewport.width),
                                        static_cast<uint32_t>(mainRenderViewport.height));

            Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(cameraTransform);
            Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(camera, mainCameraAspectRatio);
            Math::Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

            CommandList* commandList = _deferredRendering.GetCommandList();
            commandList->BeginRecording();

            if (Debug::RendererConfig::display)
            {
                if (camera.frustumCulling)
                {
                    DirectX::XMMATRIX viewProj = Math::LoadMatrix(viewMatrix) * Math::LoadMatrix(projectionMatrix);
                    _frustum.Extract(viewProj, camera.frustumPadding);
                }

                // Apply post-processing effects pre-render
                for (auto& effect : camera.postEffects)
                {
                    if (!effect->IsEnabled())
                        continue;
                    FT_ENGINE_ASSERT(effect != nullptr, "PostEffect is null");
                    effect->OnPreRender(deltaTime, viewMatrix, projectionMatrix);
                }

                _deferredRendering.BeginFrame(camera, viewMatrix, projectionMatrix, mainRenderViewport);

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

                _deferredRendering.EndFrame(camera, cameraTransform, allLights, mainRenderViewport);

                if (skyboxTexture)
                {
                    _skyboxPipeline.Render(commandList,
                                           _deferredRendering.GetFinalLitTexture(),
                                           _deferredRendering.GetDepthStencilTexture(),
                                           skyboxTexture.get(),
                                           camera,
                                           cameraTransform);
                }
            }
            else
            {
                _deferredRendering.BeginFrame(camera, viewMatrix, projectionMatrix, mainRenderViewport);
                _deferredRendering.EndFrame(camera, cameraTransform, {}, mainRenderViewport);
            }

            Texture* sceneTexture = _deferredRendering.GetFinalLitTexture();
            _ApplyPostProcessing(commandList, sceneTexture, finalRenderTarget, camera, deltaTime);

#ifdef FT_DEBUG
            if (Debug::PhysicsConfig::display && joltDebugRenderer)
            {
                joltDebugRenderer->Render(commandList,
                                          mainRenderViewport,
                                          viewProjectionMatrix,
                                          camera,
                                          finalRenderTarget,
                                          _deferredRendering.GetDepthStencilTexture());
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
                postProcessingPasses[i]->SetNormalTexture(_deferredRendering.GetNormalTexture());
                postProcessingPasses[i]->SetMaterialTexture(_deferredRendering.GetMaterialTexture());
                postProcessingPasses[i]->SetDepthTexture(_deferredRendering.GetDepthStencilTexture());
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
        const std::shared_ptr<Texture>& skybox,
        float overrideAspectRatio)
    {
        const VirtualCamera& camera = static_cast<const VirtualCamera&>(*camData.camera);
        const WorldTransform& cameraTransform = *camData.transform;
        Texture* renderTarget = camera.GetRenderTarget().get();
        if (!renderTarget)
            return;

        auto meshView = scene.ViewActive<StaticMesh, WorldTransform>();

        float targetWidth = static_cast<float>(renderTarget->GetWidth());
        float targetHeight = static_cast<float>(renderTarget->GetHeight());
        if (targetWidth == 0 || targetHeight == 0)
            return;

        Viewport renderViewport = { 0.0f, 0.0f, targetWidth, targetHeight };

        _deferredRendering.OnResize(static_cast<uint32_t>(targetWidth), static_cast<uint32_t>(targetHeight));

        float aspectRatio = (renderViewport.height > 0) ? (renderViewport.width / renderViewport.height) : 1.0f;
        if (overrideAspectRatio > 0.0f)
        {
            aspectRatio = overrideAspectRatio;
        }

        Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(cameraTransform);
        Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(camera, aspectRatio);

        CommandList* commandList = _deferredRendering.GetCommandList();
        commandList->BeginRecording();

        _deferredRendering.BeginFrame(camera, viewMatrix, projectionMatrix, renderViewport);

        meshView.each(
            [&](const StaticMesh& staticMesh, const WorldTransform& meshTransform)
            {
                if (staticMesh.GetModel())
                {
                    Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                    _deferredRendering.SubmitModel(*staticMesh.GetModel(), worldMatrix);
                }
            });

        _deferredRendering.EndFrame(camera, cameraTransform, allLights, renderViewport, renderTarget);

        if (skybox)
        {
            _skyboxPipeline.Render(commandList,
                                   renderTarget,
                                   _deferredRendering.GetDepthStencilTexture(),
                                   skybox.get(),
                                   camera,
                                   cameraTransform);
        }

        commandList->EndRecording();
        commandList->Execute();
    }
} // namespace Frost