#pragma once

#include "Frost/Renderer/GPUResource.h"
#include "Frost/Event/Events/Window/WindowResizeEvent.h"

#include <cstdint>

namespace Frost
{
	class Pipeline : public GPUResource
	{
	public:
		Pipeline() = default;
		virtual ~Pipeline() = default;

		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void OnWindowResize(WindowResizeEvent& resizeEvent) {};
	};
}