#pragma once

#include "Core/Window.h"
#include "Renderer/Device.h"

#include <memory>

namespace Frost
{
	class Window;
	class Device;

	class Renderer
	{
	public:
		Renderer(Window* window);
		void DrawFrame();

	private:
		Window* _window;
		std::unique_ptr<Device> _device;
	};
}


