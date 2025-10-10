#pragma once

#include "Frost/Core/Window.h"
#include "Frost/Renderer/Device.h"

#include <memory>

namespace Frost
{
	class Window;
	class Device;

	class Renderer
	{
	public:
		Renderer();
		void Init();
		void DrawFrame();
		void ClearColor(float r, float g, float b, float a = 1.0f);

	private:
		std::shared_ptr<Window> _window;
		std::unique_ptr<Device> _device;
	};
}


