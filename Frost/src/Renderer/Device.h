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
		void DrawFrame();

	private:
		void _CreateDevice();
		void _CreateRenderTargetView();
		void _CreateRasterizer();
		void _CreateDepthBuffer();

		Window* _window;
		IDXGISwapChain* _swapChain;
		ID3D11Device* _device;
		ID3D11DeviceContext* _immediateContext;
		ID3D11RasterizerState* _rasterizerState;
		ID3D11Texture2D* _depthTexture;
		ID3D11DepthStencilView* _depthStencilView;
		ID3D11RenderTargetView* _renderTargetView;
	};
}

