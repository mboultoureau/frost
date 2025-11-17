#include "Frost/Scene/Systems/JoltRendererSystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Physics/Physics.h"

namespace Frost
{
    JoltRendererSystem::JoltRendererSystem()
    {
    }

    void JoltRendererSystem::LateUpdate(ECS& ecs, float deltaTime)
    {
        Window::WindowSizeUnit windowWidth = Application::GetWindow()->GetWidth();
        Window::WindowSizeUnit windowHeight = Application::GetWindow()->GetHeight();
        if (windowWidth == 0 || windowHeight == 0)
        {
            return;
        }

        const auto& cameras = ecs.GetDataArray<Component::Camera>();
        const auto& cameraEntities = ecs.GetIndexMap<Component::Camera>();
        if (cameras.empty()) return;

		const Component::Camera& mainCamera = cameras[0];
		GameObject::Id mainCameraId = cameraEntities[0];
		const Component::WorldTransform* cameraTransform = ecs.GetComponent<Component::WorldTransform>(mainCameraId);
        if (!cameraTransform) return;
        

		JoltRenderingPipeline* _joltDebugRendering = static_cast<JoltRenderingPipeline*>(Physics::GetDebugRenderer());

        _joltDebugRendering->BeginFrame(mainCamera, *cameraTransform);
		_joltDebugRendering->EndFrame();
    }
}