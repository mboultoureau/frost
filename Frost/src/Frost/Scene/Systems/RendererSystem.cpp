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
#include <Frost/Renderer/Sampler.h>
#include <Frost/Renderer/DX11/SamplerDX11.h>

using namespace Frost::Component;

namespace Frost
{
    RendererSystem::RendererSystem() : _frustum{}
    {
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .filePath = "../Frost/resources/shaders/VS_FullScreenQuad.hlsl" };

        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .filePath = "../Frost/resources/shaders/PS_FullScreenQuad.hlsl" };

        _vsFullScreenQuad = Shader::Create(vsDesc);
        _psFullScreenQuad = Shader::Create(psDesc);

        _sampler = std::make_unique<SamplerDX11>(SamplerConfig{ .filter = Filter::ANISOTROPIC });
    }

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
                    /*const auto& referenceTransform =
                        scene.GetRegistry().get<WorldTransform>(relativeView.referenceEntity);

                    Math::Matrix4x4 matPlayer = Math::GetTransformMatrix(*camData.transform);
                    Math::Matrix4x4 matRef = Math::GetTransformMatrix(referenceTransform);
                    Math::Matrix4x4 matOut = Math::GetTransformMatrix(*virtualCamData.transform);

                    Math::Matrix4x4 matRel = matPlayer * Math::Matrix4x4::Invert(matRef);
                    Math::Matrix4x4 matNew = matRel * relativeView.modifier * matOut;

                    // Modifier temporairement la transformation de la camera virtuelle
                    auto& camTransform = scene.GetRegistry().get<WorldTransform>(virtualCamData.entity);
                    Math::DecomposeTransform(
                        matNew, camTransform.position, camTransform.rotation, camTransform.scale);*/
                }
            }

            _RenderSceneToTexture(scene,
                                  virtualCamData,
                                  deltaTime,
                                  allLights,
                                  skyboxTexture,
                                  aspectRatioToUse,
                                  virtualCam.portalEntity,
                                  virtualCam.linkedPortalEntity);
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

            _deferredRendering.BeginFrame(camera, cameraTransform, viewMatrix, projectionMatrix, mainRenderViewport);

            meshView.each(
                [&](const entt::entity entity, const StaticMesh& staticMesh, const WorldTransform& meshTransform)
                {
                    // AJOUT : Skip les objets avec ScreenProjectedTexture si hideGeometry est true
                    if (scene.GetRegistry().all_of<ScreenProjectedTexture>(entity))
                    {
                        const auto& projTex = scene.GetRegistry().get<ScreenProjectedTexture>(entity);
                        if (projTex.hideGeometry)
                            return; // Ne pas rendre ce mesh normalement
                    }

                    if (staticMesh.GetModel())
                    {
                        Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                        if (!camera.frustumCulling || staticMesh.overrideFrustumCulling ||
                            _IsVisible(staticMesh, worldMatrix))
                        {
                            _deferredRendering.SubmitModel(*staticMesh.GetModel(), worldMatrix);
                        }
                    }
                });

            _shadowPipeline.SetGBufferData(&_deferredRendering, &scene);
            _shadowPipeline.ShadowPass(allLights, camera, cameraTransform, camera.viewport);
            _shadowPipeline.LightPass(camera, cameraTransform, camera.viewport);

            // === RENDRE LA SKYBOX PRINCIPALE AVANT LES PORTAILS ===
            if (skyboxTexture)
            {
                Texture* finalTarget = _deferredRendering.GetFinalLitTexture();
                commandList->SetRenderTargets(1, &finalTarget, _deferredRendering.GetDepthStencilTexture());

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
            _deferredRendering.BeginFrame(camera, cameraTransform, viewMatrix, projectionMatrix, mainRenderViewport);
            _shadowPipeline.SetGBufferData(&_deferredRendering, &scene);
            _shadowPipeline.ShadowPass(allLights, camera, cameraTransform, camera.viewport);
            _shadowPipeline.LightPass(camera, cameraTransform, camera.viewport);
        }

        Texture* sceneTexture = _shadowPipeline.GetFinalLitTexture();
        _ApplyPostProcessing(commandList, sceneTexture, finalRenderTarget, camera, deltaTime);

        // === PUIS RENDRE LES PORTAILS PAR-DESSUS ===
        _RenderScreenProjectedTexturesOnTarget(scene, camData, mainRenderViewport, finalRenderTarget);

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
} // namespace Frost

bool
RendererSystem::_IsVisible(const Component::StaticMesh& staticMesh, const Math::Matrix4x4& worldMatrix)
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

        // mesh.enabled = renderMesh;
    }
    return renderModel;
}

void
RendererSystem::_RenderScreenProjectedTexturesOnTarget(Scene& scene,
                                                       const RenderCameraData& camData,
                                                       const Viewport& viewport,
                                                       Texture* targetTexture)
{
    auto view = scene.ViewActive<ScreenProjectedTexture, StaticMesh, WorldTransform>();
    if (view.size_hint() == 0)
        return;

    CommandList* commandList = _deferredRendering.GetCommandList();

    // Bind la target finale (après post-process)
    commandList->SetRenderTargets(1, &targetTexture, _deferredRendering.GetDepthStencilTexture());
    commandList->SetViewport(viewport.x, viewport.y, viewport.width, viewport.height, 0.0f, 1.0f);

    Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(*camData.transform);
    Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(*camData.camera, viewport.width / viewport.height);

    view.each(
        [&](const entt::entity entity,
            const ScreenProjectedTexture& projectedTex,
            const StaticMesh& mesh,
            const WorldTransform& transform)
        {
            if (!projectedTex.enabled || !projectedTex.texture)
                return;

            // Clear stencil
            commandList->ClearDepthStencil(_deferredRendering.GetDepthStencilTexture(), false, 1.0f, true, 0);

            // ecrire le mesh dans le stencil
            commandList->SetColorWriteMask(false, false, false, false);
            commandList->SetDepthStencilStateCustom(true,
                                                    false,
                                                    CompareFunction::LessEqual,
                                                    true,
                                                    CompareFunction::Always,
                                                    StencilOp::Keep,
                                                    StencilOp::Keep,
                                                    StencilOp::Replace,
                                                    1,
                                                    0xFF,
                                                    0xFF);

            if (mesh.GetModel())
            {
                Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(transform);
                _deferredRendering.SubmitModelStencilOnly(*mesh.GetModel(), worldMatrix, viewMatrix, projectionMatrix);
            }

            // Rendre la texture avec stencil test
            commandList->SetColorWriteMask(true, true, true, true);
            commandList->SetDepthStencilStateCustom(false,
                                                    false,
                                                    CompareFunction::Always,
                                                    true,
                                                    CompareFunction::Equal,
                                                    StencilOp::Keep,
                                                    StencilOp::Keep,
                                                    StencilOp::Keep,
                                                    1,
                                                    0xFF,
                                                    0x00);

            commandList->SetBlendState(BlendMode::None);
            _RenderFullscreenQuad(commandList, projectedTex.texture.get());
        });

    // Restaurer les etats
    commandList->SetDepthStencilStateCustom(true,
                                            true,
                                            CompareFunction::Less,
                                            false,
                                            CompareFunction::Always,
                                            StencilOp::Keep,
                                            StencilOp::Keep,
                                            StencilOp::Keep,
                                            0,
                                            0xFF,
                                            0xFF);

    commandList->SetColorWriteMask(true, true, true, true);
    commandList->SetBlendState(BlendMode::None);
}

void
RendererSystem::_RenderFullscreenQuad(CommandList* commandList, Texture* texture)
{
    // Utilisez un shader simple qui rend une texture en fullscreen
    // Vous devrez creer ce shader s'il n'existe pas deja

    commandList->UnbindShader(ShaderType::Geometry);
    commandList->UnbindShader(ShaderType::Hull);
    commandList->UnbindShader(ShaderType::Domain);
    commandList->SetShader(_vsFullScreenQuad.get());
    commandList->SetShader(_psFullScreenQuad.get());
    commandList->SetTexture(texture, 0);
    commandList->SetSampler(_sampler.get(), 0);
    commandList->SetInputLayout(nullptr);
    commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
    commandList->Draw(3, 0); // Triangle qui couvre l'ecran
}

void
RendererSystem::_ApplyPostProcessing(CommandList* commandList,
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
            destination = (i == postProcessingPasses.size() - 1) ? destinationTarget
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

void
RendererSystem::_RenderSceneToTexture(
    Scene& scene,
    const RenderCameraData& camData,
    float deltaTime,
    const std::vector<std::pair<Component::Light, Component::WorldTransform>>& allLights,
    const std::shared_ptr<Texture>& skybox,
    float overrideAspectRatio,
    entt::entity exitPortalFrameEntity,
    entt::entity entryPortalFrameEntity) // Portail a utiliser pour le masque
{
    // if (entryPortalFrameEntity == entt::null)
    //     return;

    const VirtualCamera& camera = static_cast<const VirtualCamera&>(*camData.camera);
    const WorldTransform& cameraTransform = *camData.transform;
    Texture* renderTarget = scene.GetRegistry().get<ScreenProjectedTexture>(entryPortalFrameEntity).texture.get();
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

    CommandList* commandList = _deferredRendering.GetCommandList();
    commandList->BeginRecording();

    commandList->ClearDepthStencil(_deferredRendering.GetDepthStencilTexture(), true, 1.0f, true, 0);

    // === eTAPE 1 : Rendre le masque du portail dans le stencil ===
    if (scene.GetRegistry().valid(exitPortalFrameEntity))
    {
        // IMPORTANT : Binder un render target dummy + le depth/stencil
        Texture* dummyRT = _deferredRendering.GetAlbedoTexture(); // N'importe quel RT
        commandList->SetRenderTargets(1, &dummyRT, _deferredRendering.GetDepthStencilTexture());

        // Viewport
        commandList->SetViewport(0, 0, targetWidth, targetHeight, 0.0f, 1.0f);

        // Desactiver l'ecriture couleur
        commandList->SetColorWriteMask(false, false, false, false);
        commandList->SetDepthStencilStateCustom(false, // depthEnable
                                                false, // depthWrite
                                                CompareFunction::Always,
                                                true, // stencilEnable
                                                CompareFunction::Always,
                                                StencilOp::Keep,
                                                StencilOp::Keep,
                                                StencilOp::Replace,
                                                1,
                                                0xFF,
                                                0xFF);

        if (scene.GetRegistry().all_of<StaticMesh, WorldTransform>(exitPortalFrameEntity))
        {
            const auto& portalMesh = scene.GetRegistry().get<StaticMesh>(exitPortalFrameEntity);
            const auto& portalTransform = scene.GetRegistry().get<WorldTransform>(exitPortalFrameEntity);

            if (portalMesh.GetModel())
            {
                Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(portalTransform);
                _deferredRendering.SubmitModelStencilOnly(
                    *portalMesh.GetModel(), worldMatrix, viewMatrix, projectionMatrix);
            }
        }

        // Reactiver l'ecriture couleur
        commandList->SetColorWriteMask(true, true, true, true);
    }

    // === eTAPE 2 : BeginFrame (va re-binder les render targets) ===
    _deferredRendering.BeginFrame(camera, viewMatrix, projectionMatrix, renderViewport);
    //_deferredRendering.BeginFrame(camera, viewMatrix, obliqueProjection, renderViewport);
    // === eTAPE 3 : Configurer le stencil pour le GBuffer pass ===
    if (scene.GetRegistry().valid(exitPortalFrameEntity))
    {
        commandList->SetDepthStencilStateCustom(true,                   // depthEnable
                                                true,                   // depthWrite
                                                CompareFunction::Less,  // depthFunc
                                                true,                   // stencilEnable
                                                CompareFunction::Equal, // REMIS a Equal : rendre où stencil == 1
                                                StencilOp::Keep,
                                                StencilOp::Keep,
                                                StencilOp::Keep,
                                                1, // stencilRef
                                                0xFF,
                                                0x00);
    }

    // === ÉTAPE 4 : Rendre la geometrie ===
    meshView.each(
        [&](const entt::entity entity, const StaticMesh& staticMesh, const WorldTransform& meshTransform)
        {
            if (entity == exitPortalFrameEntity)
                return;

            if (staticMesh.GetModel())
            {
                Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                _deferredRendering.SubmitModel(*staticMesh.GetModel(), worldMatrix);
            }
        });

    // === eTAPE 5 : EndFrame SANS stencil ===
    _deferredRendering.EndFrame(camera, cameraTransform, allLights, renderViewport, renderTarget);

    // === eTAPE 6 : Skybox - SANS stencil pour remplir toute la texture ===
    if (skybox)
    {
        // Desactiver le stencil pour la skybox
        commandList->SetDepthStencilStateCustom(true,                       // depthEnable
                                                false,                      // depthWrite (skybox n'ecrit pas le depth)
                                                CompareFunction::LessEqual, // depthFunc
                                                false,
                                                CompareFunction::Always,
                                                StencilOp::Keep,
                                                StencilOp::Keep,
                                                StencilOp::Keep,
                                                0,
                                                0xFF,
                                                0xFF);

        _skyboxPipeline.Render(commandList,
                               renderTarget.get(),
                               _deferredRendering.GetDepthStencilTexture().get(),
                               skyboxTexture.get(),
                               camera,
                               cameraTransform);
    }

    // === eTAPE 7 : Appliquer les post-effets de la camera virtuelle ===
    if (!camera.postEffects.empty())
    {
        // Creer une texture temporaire pour les post-effets du portail
        uint32_t rtWidth = renderTarget->GetWidth();
        uint32_t rtHeight = renderTarget->GetHeight();

        // Reutiliser ou creer des textures temporaires
        if (!_portalPostProcessSource || _portalPostProcessSource->GetWidth() != rtWidth ||
            _portalPostProcessSource->GetHeight() != rtHeight)
        {
            TextureConfig ppConfig = { .format = renderTarget->GetFormat(),
                                       .width = rtWidth,
                                       .height = rtHeight,
                                       .isRenderTarget = true,
                                       .isShaderResource = true };
            _portalPostProcessSource = Texture::Create(ppConfig);
            _portalPostProcessDestination = Texture::Create(ppConfig);
        }

        // Copier le rendu actuel dans la source
        commandList->CopyResource(_portalPostProcessSource.get(), renderTarget);

        // Appliquer les post-effets
        _ApplyVirtualCameraPostProcessing(commandList, _portalPostProcessSource.get(), renderTarget, camera, deltaTime);
    }

    commandList->EndRecording();
    commandList->Execute();
}

void
RendererSystem::_ApplyVirtualCameraPostProcessing(CommandList* commandList,
                                                  Texture* sourceTexture,
                                                  Texture* destinationTarget,
                                                  const VirtualCamera& camera,
                                                  float deltaTime)
{
    std::vector<std::shared_ptr<PostEffect>> activeEffects;
    for (const auto& effect : camera.postEffects)
    {
        if (effect && effect->IsEnabled() && effect->IsPostProcessingPass())
        {
            activeEffects.push_back(effect);
        }
    }

    if (activeEffects.empty())
        return;

    Texture* source = sourceTexture;
    Texture* destination = nullptr;

    for (size_t i = 0; i < activeEffects.size(); ++i)
    {
        // La dernière passe va sur la destination finale
        destination = (i == activeEffects.size() - 1)
                          ? destinationTarget
                          : ((i % 2 == 0) ? _portalPostProcessDestination.get() : _portalPostProcessSource.get());

        // Configurer les textures pour le post-effet
        activeEffects[i]->SetNormalTexture(_deferredRendering.GetNormalTexture());
        activeEffects[i]->SetMaterialTexture(_deferredRendering.GetMaterialTexture());
        activeEffects[i]->SetDepthTexture(_deferredRendering.GetDepthStencilTexture());

        // Appliquer l'effet
        activeEffects[i]->OnPostRender(deltaTime, commandList, source, destination);

        source = destination;
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