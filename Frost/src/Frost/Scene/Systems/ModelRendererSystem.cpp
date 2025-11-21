#include "Frost/Scene/Systems/ModelRendererSystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Debugging/DebugInterface/DebugRendering.h"
#include "Frost/Scene/Components/Skybox.h"
#include "Frost/Renderer/RendererAPI.h"

using namespace Frost::Component;

namespace Frost
{
    ModelRendererSystem::ModelRendererSystem()
    {
    }

    void ModelRendererSystem::LateUpdate(Scene& scene, float deltaTime)
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

        std::shared_ptr<Texture> skyboxTexture{};

        auto cameraView = scene.ViewActive<Camera, WorldTransform>();
        auto lightView = scene.ViewActive<Light, WorldTransform>();
        auto skyboxView = scene.ViewActive<Skybox>();
        auto meshView = scene.ViewActive<StaticMesh, WorldTransform>();

        std::vector<std::pair<Component::Light, Component::WorldTransform>> allLights;
        allLights.reserve(lightView.size_hint());
        for (auto entity : lightView)
        {
            const auto& [light, transform] = lightView.get<Component::Light, Component::WorldTransform>(entity);
            allLights.emplace_back(light, transform);
        }

		// Skybox (take only the first one but to be changed when portal rendering is implemented)
        if (skyboxView.begin() != skyboxView.end())
        {
            const auto& skyboxEntity = *skyboxView.begin();
			const auto& [skybox] = skyboxView.get(skyboxEntity);
            skyboxTexture = skybox.cubemapTexture;
        }

        struct RenderCamera {
            entt::entity entity;
            const Camera* camera;
            const WorldTransform* transform;
        };

        std::vector<RenderCamera> sortedCameras;
        sortedCameras.reserve(cameraView.size_hint());

        for (auto entity : cameraView)
        {
            const auto& [camera, transform] = cameraView.get(entity);
            sortedCameras.push_back({ entity, &camera, &transform });
        }

		// Sort by priority
        std::sort(sortedCameras.begin(), sortedCameras.end(),
            [](const RenderCamera& a, const RenderCamera& b)
            {
                return a.camera->priority < b.camera->priority;
            }
        );

        for (const auto& camData : sortedCameras)
        {
            entt::entity entity = camData.entity;
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
            const float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
            const float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());
            const float viewportWidth = camera.viewport.width * windowWidth;
            const float viewportHeight = camera.viewport.height * windowHeight;
            const float aspectRatio = (viewportHeight > 0) ? (viewportWidth / viewportHeight) : 1.0f;

            Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(cameraTransform);
            Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(camera, aspectRatio);

			// Apply post-processing effects pre-render
            for (auto& effect : activeEffects)
            {
				FT_ENGINE_ASSERT(effect != nullptr, "PostEffect is null");
                effect->OnPreRender(deltaTime, viewMatrix, projectionMatrix);
            }

            // Draw meshes
            _deferredRendering.BeginFrame(camera, viewMatrix, projectionMatrix);

            for (auto meshEntity : meshView)
            {
                const auto& [staticMesh, meshTransform] = meshView.get(meshEntity);

                if (staticMesh.model)
                {
                    Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(meshTransform);
                    _deferredRendering.SubmitModel(*staticMesh.model, worldMatrix);
                }
            }

            _deferredRendering.EndFrame(camera, cameraTransform, allLights);

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
						// The last effect renders to the back buffer
                        destination = RendererAPI::GetRenderer()->GetBackBuffer();
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
                commandList->CopyResource(RendererAPI::GetRenderer()->GetBackBuffer(), sceneTexture);
            }

            commandList->EndRecording();
            commandList->Execute();
            RendererAPI::GetRenderer()->RestoreBackBufferRenderTarget();
        }
    }
}