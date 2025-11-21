#include "Frost/Scene/Systems/JoltRendererSystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Physics/Physics.h"
#include "Frost/Debugging/DebugInterface/DebugPhysics.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/WorldTransform.h"

using namespace Frost::Component;

namespace Frost
{
    JoltRendererSystem::JoltRendererSystem()
    {
    }

    void JoltRendererSystem::LateUpdate(Scene& scene, float deltaTime)
    {
#ifdef FT_DEBUG
        if (!Debug::PhysicsConfig::display)
        {
            JoltRenderingPipeline* pipeline = static_cast<JoltRenderingPipeline*>(Physics::GetDebugRenderer());
            if (pipeline)
            {
                pipeline->Clear();
            }
            return;
        }
#endif

        Window::WindowSizeUnit windowWidth = Application::GetWindow()->GetWidth();
        Window::WindowSizeUnit windowHeight = Application::GetWindow()->GetHeight();
        if (windowWidth == 0 || windowHeight == 0)
        {
            return;
        }

        JoltRenderingPipeline* _joltDebugRendering = static_cast<JoltRenderingPipeline*>(Physics::GetDebugRenderer());
        if (!_joltDebugRendering) return;

        auto cameraView = scene.ViewActive<Component::Camera, Component::WorldTransform>();

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

        std::sort(sortedCameras.begin(), sortedCameras.end(),
            [](const RenderCamera& a, const RenderCamera& b)
            {
                return a.camera->priority < b.camera->priority;
            }
        );

        for (const auto& camData : sortedCameras)
        {
            const Camera& camera = *camData.camera;
            const WorldTransform& cameraTransform = *camData.transform;

            _joltDebugRendering->Render(camera, cameraTransform);
        }

        _joltDebugRendering->Clear();
    }
}