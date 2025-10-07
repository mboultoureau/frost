#include "Renderer.h"

namespace Frost
{
	Renderer::Renderer(Window* window) : _window { window }
	{
		_device = std::make_unique<Device>(_window);
	}
}
