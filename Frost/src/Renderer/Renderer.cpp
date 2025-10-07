#include "Renderer.h"

#include <cassert>

namespace Frost
{
	Renderer::Renderer(Window* window) : _window { window }
	{
		_device = std::make_unique<Device>(_window);
	}

	void Renderer::DrawFrame()
	{
		assert(_device != nullptr);
		_device->DrawFrame();
	}
}
