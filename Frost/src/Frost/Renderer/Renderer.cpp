#include "Frost/Renderer/Renderer.h"

#include "Frost/Event/Events/Window/WindowResizeEvent.h"
#include "Frost/Debugging/Assert.h"

namespace Frost
{
	Renderer::Renderer()
	{
		_resizeEventHandlerId = EventManager::Subscribe<WindowResizeEvent>([this](auto& e) {

			for (Pipeline* pipeline : _pipelines)
			{
				pipeline->Shutdown();
			}

			OnWindowResize(e);

			for (Pipeline* pipeline : _pipelines)
			{
				pipeline->Initialize();
			}


			return false;
		});
	}

	Renderer::~Renderer()
	{
		EventManager::Unsubscribe<WindowResizeEvent>(_resizeEventHandlerId);
	}

	void Renderer::RegisterPipeline(Pipeline* pipeline)
	{
		_pipelines.push_back(pipeline);
	}

	void Renderer::UnregisterPipeline(Pipeline* pipeline)
	{
		auto it = std::remove_if(_pipelines.begin(), _pipelines.end(), [&pipeline](Pipeline* p) {
			return p == pipeline;
		});
		FT_ENGINE_ASSERT(it != _pipelines.end(), "Pipeline to unregister not found in renderer!");
		_pipelines.erase(it, _pipelines.end());
	}
}
