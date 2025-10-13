#pragma once

#include "Frost/Core/Window.h"

#include <memory>
#include <d3d11.h>
#include <wrl/client.h>

namespace Frost
{
	class Window;

	class Device
	{
	public:
		Device(Window* window);
		~Device();

		void HandleWindowResize(UINT width, UINT height);

	private:
		void _CreateDevice();
		void _CreateRasterizer();
		void _CreateViewsAndViewport(UINT width, UINT height);
		void _CreateDepthBuffer(UINT width, UINT height);
		void _SetupDebug();
		void _ReportLiveObjects();
		void _ReleaseViews();

		Window* _window;
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain{};
		Microsoft::WRL::ComPtr<ID3D11Device> _device{};
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _immediateContext{};
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState{};
		Microsoft::WRL::ComPtr<ID3D11Texture2D> _depthTexture{};
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthStencilView{};
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _renderTargetView{};

		friend class Renderer;
	};
}

