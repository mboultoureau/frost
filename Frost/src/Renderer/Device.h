#pragma once

#include "Core/Window.h"

#include <memory>
#include <d3d11.h>

namespace Frost
{
	class Window;

	class Device
	{
	public:
		Device(Window* window);

	private:
		void _CreateDevice();

		Window* _window;
		IDXGISwapChain* _swapChain;
		ID3D11Device* _device;
		ID3D11DeviceContext* _immediateContext;
	};
}

