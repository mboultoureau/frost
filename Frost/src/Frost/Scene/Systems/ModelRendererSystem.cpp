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

            _deferredRendering.BeginFrame(currentCamera, *cameraTransform);

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
                _skyboxPipeline.Render(currentCamera, *cameraTransform, gbufferDepth, activeSkyboxTexture.get());
            }
        }

    }
}