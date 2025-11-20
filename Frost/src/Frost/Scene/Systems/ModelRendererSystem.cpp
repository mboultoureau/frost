#include "Frost/Scene/Systems/ModelRendererSystem.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Light.h"
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

    void ModelRendererSystem::LateUpdate(Frost::ECS& ecs, float deltaTime)
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

        const auto& cameras = ecs.GetDataArray<Component::Camera>();
        const auto& cameraEntities = ecs.GetIndexMap<Component::Camera>();
        if (cameras.empty()) return;

        const auto& lights = ecs.GetDataArray<Component::Light>();
        const auto& lightEntities = ecs.GetIndexMap<Component::Light>();

        std::vector<std::pair<Component::Light, Component::WorldTransform>> allLights;
        allLights.reserve(lights.size());

        for (size_t i = 0; i < lights.size(); ++i)
        {
            const Component::Light& lightComponent = lights[i];
            GameObject::Id lightId = lightEntities[i];

            const Component::WorldTransform* transform = ecs.GetComponent<Component::WorldTransform>(lightId);

            if (transform)
            {
                allLights.emplace_back(lightComponent, *transform);
            }
        }

		// Skybox (take only the first one but to be changed when portal rendering is implemented)
        auto& skyboxes = ecs.GetDataArray<Component::Skybox>();
        std::shared_ptr<Texture> activeSkyboxTexture = nullptr;

        if (!skyboxes.empty())
        {
            auto& sceneSkybox = skyboxes[0];
            activeSkyboxTexture = sceneSkybox.cubemapTexture;
        }

        const auto& staticMeshes = ecs.GetDataArray<Component::StaticMesh>();
        const auto& staticMeshesEntities = ecs.GetIndexMap<Component::StaticMesh>();

        for (size_t i = 0; i < cameras.size(); ++i)
        {
            const Component::Camera& currentCamera = cameras[i];
            GameObject::Id cameraId = cameraEntities[i];
            const Component::WorldTransform* cameraTransform = ecs.GetComponent<Component::WorldTransform>(cameraId);

            if (!cameraTransform)
            {
                continue;
            }

            // Get only active effects
            std::vector<std::shared_ptr<PostEffect>> activeEffects;
            for (const auto& effect : currentCamera.postEffects)
            {
                if (effect && effect->IsEnabled())
                {
                    activeEffects.push_back(effect);
                }
            }

			// Calculate view and projection matrices
            const float windowWidth = static_cast<float>(Application::GetWindow()->GetWidth());
            const float windowHeight = static_cast<float>(Application::GetWindow()->GetHeight());
            const float viewportWidth = currentCamera.viewport.width * windowWidth;
            const float viewportHeight = currentCamera.viewport.height * windowHeight;
            const float aspectRatio = (viewportHeight > 0) ? (viewportWidth / viewportHeight) : 1.0f;

            Math::Matrix4x4 viewMatrix = Math::GetViewMatrix(*cameraTransform);
            Math::Matrix4x4 projectionMatrix = Math::GetProjectionMatrix(currentCamera, aspectRatio);

			// Apply post-processing effects pre-render
            for (auto& effect : activeEffects)
            {
				FT_ENGINE_ASSERT(effect != nullptr, "PostEffect is null");
                effect->OnPreRender(deltaTime, viewMatrix, projectionMatrix);
            }

            // Draw meshes
            _deferredRendering.BeginFrame(currentCamera, viewMatrix, projectionMatrix);

            for (size_t i = 0; i < staticMeshes.size(); ++i)
            {
                const StaticMesh& staticMesh = staticMeshes[i];
                GameObject::Id entityId = staticMeshesEntities[i];

                const Component::WorldTransform* transform = ecs.GetComponent<Component::WorldTransform>(entityId);

                if (transform && staticMesh.model && staticMesh.isActive)
                {
                    Math::Matrix4x4 worldMatrix = Math::GetTransformMatrix(*transform);
                    _deferredRendering.SubmitModel(*staticMesh.model, worldMatrix);
                }
            }

            _deferredRendering.EndFrame(currentCamera, *cameraTransform, allLights);

			// Skybox rendering
            if (activeSkyboxTexture)
            {
                Texture* gbufferDepth = _deferredRendering.GetDepthStencilTexture();
                Texture* finalLitTexture = _deferredRendering.GetFinalLitTexture();
                CommandList* commandList = _deferredRendering.GetCommandList();

                _skyboxPipeline.Render(commandList, finalLitTexture, gbufferDepth, activeSkyboxTexture.get(), currentCamera, *cameraTransform);
            }

            CommandList* commandList = _deferredRendering.GetCommandList();
            Texture* sceneTexture = _deferredRendering.GetFinalLitTexture();

			// Filter through post-processing effects
            std::vector<std::shared_ptr<PostEffect>> postProcessingPasses;
            for (const auto& effect : currentCamera.postEffects)
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