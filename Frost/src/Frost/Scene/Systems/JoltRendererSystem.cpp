#include "Frost/Scene/Systems/JoltRendererSystem.h"
#include "Frost/Core/Application.h"
#include "Frost/Physics/Physics.h"

namespace Frost
{
    JoltRendererSystem::JoltRendererSystem()
    {
    }

    void JoltRendererSystem::LateUpdate(Scene& scene, float deltaTime)
    {
        Window::WindowSizeUnit windowWidth = Application::GetWindow()->GetWidth();
        Window::WindowSizeUnit windowHeight = Application::GetWindow()->GetHeight();
        if (windowWidth == 0 || windowHeight == 0)
        {
            return;
        }

		auto cameraView = scene.ViewActive<Component::Camera, Component::WorldTransform>();

		JoltRenderingPipeline* _joltDebugRendering = static_cast<JoltRenderingPipeline*>(Physics::GetDebugRenderer());

		// Get the first active camera
		auto entity = cameraView.front();
		const auto& [camera, cameraTransform] = cameraView.get<Component::Camera, Component::WorldTransform>(entity);

        _joltDebugRendering->BeginFrame(camera, cameraTransform);
		_joltDebugRendering->EndFrame();
    }
}