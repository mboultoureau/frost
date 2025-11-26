#include "Frost/Scene/Systems/RendererSystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Debugging/DebugInterface/DebugRendering.h"
#include "Frost/Scene/Components/Skybox.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Scene/Components/RelativeView.h"

using namespace Frost::Component;

namespace Frost
{
    RendererSystem::RendererSystem()
    {
    }

    void RendererSystem::LateUpdate(Scene& scene, float deltaTime)
    {
#ifdef FT_DEBUG
        if (!Debug::RendererConfig::display)
        {
            return;
        }
#endif

        Window::WindowSizeUnit windowWidth = Application::GetWindow()->GetWidth();
        Window::WindowSizeUnit windowHeight = Application::GetWindow()->GetHeight();
        if (windowWidth == 0 || windowHeight == 0)
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
        lightView.each([&](const Component::Light& light, const Component::WorldTransform& transform) {
            allLights.emplace_back(light, transform);
        });

        // Skybox (take only the first one but to be changed when portal rendering is implemented)
        std::shared_ptr<Texture> skyboxTexture{};
        skyboxView.each([&](const Skybox& skybox) {
            if (!skyboxTexture) {
                skyboxTexture = skybox.cubemapTexture;
            }
        });

        struct RenderCamera {
            entt::entity entity;
            const Camera* camera;
            const WorldTransform* transform;
        };

        std::vector<RenderCameraData> virtualCameras;
        std::vector<RenderCameraData> classicCameras;
        virtualCameras.reserve(virtualCameraView.size_hint());
        classicCameras.reserve(cameraView.size_hint());

        cameraView.each([&](entt::entity entity, const Camera& camera, const WorldTransform& transform) {
            classicCameras.push_back({ entity, &camera, &transform });
        });

        virtualCameraView.each([&](entt::entity entity, const VirtualCamera& virtualCamera, const WorldTransform& transform) {
            virtualCameras.push_back({ entity, &virtualCamera, &transform });
        });

        // Sort by priority
        auto sortCam = [](const RenderCameraData& a, const RenderCameraData& b) { return a.camera->priority < b.camera->priority; };
        std::sort(virtualCameras.begin(), virtualCameras.end(), sortCam);
        std::sort(classicCameras.begin(), classicCameras.end(), sortCam);

        // Render classic cameras
        for (const auto& camData : classicCameras)
        {
			// Calculate Main Camera Aspect Ratio
			const float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
			const float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());
			Viewport renderViewport = {
				camData.camera->viewport.x * windowWidth,
				camData.camera->viewport.y * windowHeight,
				camData.camera->viewport.width * windowWidth,
				camData.camera->viewport.height * windowHeight
			};
			const float mainCameraAspectRatio = (renderViewport.height > 0) ? (renderViewport.width / renderViewport.height) : 1.0f;

			// Render virtual cameras for this camera
			std::unordered_map<entt::entity, std::shared_ptr<Texture>> virtualCameraTargets;

			for (const auto& virtualCamData : virtualCameras)
			{
				const VirtualCamera& virtualCam = static_cast<const VirtualCamera&>(*virtualCamData.camera);
				Texture* renderTarget = virtualCam.GetRenderTarget().get();

				float aspectRatioToUse = 0.0f;
				if (virtualCam.useScreenSpaceAspectRatio)
				{
					aspectRatioToUse = mainCameraAspectRatio;
				}

				// Check if this virtual camera has a relative view
				if (scene.GetRegistry().all_of<RelativeView>(virtualCamData.entity))
				{
					const auto& relativeView = scene.GetRegistry().get<RelativeView>(virtualCamData.entity);
					if (scene.GetRegistry().valid(relativeView.referenceEntity))
					{
						const auto& referenceTransform = scene.GetRegistry().get<WorldTransform>(relativeView.referenceEntity);
						const auto& exitTransform = *virtualCamData.transform;
						const auto& playerTransform = *camData.transform;

						Math::Matrix4x4 M_player = Math::GetTransformMatrix(playerTransform);
						Math::Matrix4x4 M_ref = Math::GetTransformMatrix(referenceTransform);
						Math::Matrix4x4 M_out = Math::GetTransformMatrix(exitTransform);

						Math::Matrix4x4 M_rel = M_player * Math::Matrix4x4::Invert(M_ref);
						Math::Matrix4x4 M_new = M_rel * relativeView.modifier * M_out;

						DirectX::XMMATRIX mNew = Math::LoadMatrix(M_new);
						DirectX::XMVECTOR scale, rot, pos;
						DirectX::XMMatrixDecompose(&scale, &rot, &pos, mNew);

						auto& camTransform = scene.GetRegistry().get<WorldTransform>(virtualCamData.entity);
						camTransform.position = Math::vector_cast<Math::Vector3>(pos);
						camTransform.rotation = Math::vector_cast<Math::Vector4>(rot);
					}
				}

				_RenderSceneForCamera(scene, virtualCamData, deltaTime, allLights, skyboxTexture, renderTarget, aspectRatioToUse);
				virtualCameraTargets[virtualCamData.entity] = virtualCam.GetRenderTarget();
				
				if (scene.GetRegistry().all_of<RelativeView>(virtualCamData.entity))
				{
					auto& camTransform = scene.GetRegistry().get<WorldTransform>(virtualCamData.entity);
					camTransform = *virtualCamData.transform;
				}
			}

			// Apply materials that use virtual camera textures
            meshView.each([&](StaticMesh& staticMesh, const WorldTransform& /*unused*/)
            {
                if (staticMesh.model)
                {
                    for (auto& material : staticMesh.model->GetMaterials())
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

            _RenderSceneForCamera(scene, camData, deltaTime, allLights, skyboxTexture, nullptr);
        }
    }

    void RendererSystem::_RenderSceneForCamera(
		Scene& scene,
        const RenderCameraData& camData,
        float deltaTime,
        const std::vector<std::pair<Light, WorldTransform>>& allLights,
        const std::shared_ptr<Texture>& skyboxTexture,
        Texture* overrideRenderTarget,
        float overrideAspectRatio)
    {
		const auto& meshView = scene.ViewActive<StaticMesh, WorldTransform>();
        const Camera& camera = *camData.camera;
        const WorldTransform& cameraTransform = *camData.transform;

        // Get only active effects
        std::vector<std::shared_ptr<PostEffect>> activeEffects;
        for (const auto& effect : camera.postEffects)
        {
            if (effect && effect->IsEnabled())
            {
                activeEffects.push_back(effect);
            }
        }

		// Calculate view and projection matrices
        float targetWidth, targetHeight;
        Viewport renderViewport;

        if (overrideRenderTarget)
        {
            targetWidth = static_cast<float>(overrideRenderTarget->GetWidth());
            targetHeight = static_cast<float>(overrideRenderTarget->GetHeight());
            renderViewport = { 0.0f, 0.0f, targetWidth, targetHeight };
        }
        else
        {
            const float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
            const float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());
            targetWidth = windowWidth;
            targetHeight = windowHeight;

            renderViewport = {
                camera.viewport.x * windowWidth,
                camera.viewport.y * windowHeight,
                camera.viewport.width * windowWidth,
                camera.viewport.height * windowHeight
            };
        }

        if (targetWidth == 0 || targetHeight == 0) return;
        _deferredRendering.OnResize(static_cast<uint32_t>(targetWidth), static_cast<uint32_t>(targetHeight));

        float aspectRatio = (renderViewport.height > 0) ? (renderViewport.width / renderViewport.height) : 1.0f;
        if (overrideAspectRatio > 0.0f)
        {
            aspectRatio = overrideAspectRatio;
        }

        Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(cameraTransform);
        Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(camera, aspectRatio);


		// Apply post-processing effects pre-render
        for (auto& effect : activeEffects)
        {
			FT_ENGINE_ASSERT(effect != nullptr, "PostEffect is null");
            effect->OnPreRender(deltaTime, viewMatrix, projectionMatrix);
        }

        // Draw meshes
        _deferredRendering.BeginFrame(camera, viewMatrix, projectionMatrix, renderViewport);

        meshView.each([&](const StaticMesh& staticMesh, const WorldTransform& meshTransform)
        {
            if (staticMesh.model)
            {
                Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                _deferredRendering.SubmitModel(*staticMesh.model, worldMatrix);
            }
        });

        _deferredRendering.EndFrame(camera, cameraTransform, allLights, renderViewport);

		// Skybox rendering
        if (skyboxTexture)
        {
            Texture* gbufferDepth = _deferredRendering.GetDepthStencilTexture();
            Texture* finalLitTexture = _deferredRendering.GetFinalLitTexture();
            CommandList* commandList = _deferredRendering.GetCommandList();

            _skyboxPipeline.Render(commandList, finalLitTexture, gbufferDepth, skyboxTexture.get(), camera, cameraTransform);
        }

        CommandList* commandList = _deferredRendering.GetCommandList();
        Texture* sceneTexture = _deferredRendering.GetFinalLitTexture();
        Texture* finalRenderTarget = overrideRenderTarget ? overrideRenderTarget : RendererAPI::GetRenderer()->GetBackBuffer();

		// Filter through post-processing effects
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
            uint32_t width = Application::GetWindow()->GetWidth();
            uint32_t height = Application::GetWindow()->GetHeight();
            if (!_source || _source->GetWidth() != width || _source->GetHeight() != height)
            {
                TextureConfig ppConfig = { .format = Format::RGBA8_UNORM,.width = width, .height = height, .isRenderTarget = true, .isShaderResource = true };
                _source.reset(new TextureDX11(ppConfig));
                _destination.reset(new TextureDX11(ppConfig));
            }

            Texture* source = sceneTexture;
            Texture* destination = nullptr;

            for (size_t i = 0; i < postProcessingPasses.size(); ++i)
            {
                if (i == postProcessingPasses.size() - 1)
                {
					// The last effect renders to the destination buffer
                    destination = finalRenderTarget;
                }
                else
                {
					// Ping-pong between the two textures
                    destination = (i % 2 == 0) ? _source.get() : _destination.get();
                }

                postProcessingPasses[i]->OnPostRender(deltaTime, commandList, source, destination);
                source = destination;
            }
        }
        else
        {
            // Check if texture are always same size
            if (finalRenderTarget->GetWidth() != sceneTexture->GetWidth() ||
                finalRenderTarget->GetHeight() != sceneTexture->GetHeight())
            {
                FT_ENGINE_WARN("Final render target size does not match scene texture size. Skipping copy.");
                return;
			}

            commandList->CopyResource(finalRenderTarget, sceneTexture);
        }

        commandList->EndRecording();
        commandList->Execute();

        if (!overrideRenderTarget)
        {
            RendererAPI::GetRenderer()->RestoreBackBufferRenderTarget();
        }
    }
}