#pragma once

#include "Frost/Core/Window.h"
#include "Frost/Event/Events/Window/WindowResizeEvent.h"
#include "Frost/Event/Events/Debugging/DebugOptionChangedEvent.h"

#include <memory>
#include <d3d11_1.h>
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
		void EnableWireframe();
		void DisableWireframe();
		bool IsWireframeEnabled() const { return _isWireframeEnabled; }

	private:
		void _CreateDevice();
		void _CreateRasterizer();
		void _CreateWireframeRasterizer();
		void _CreateViewsAndViewport(UINT width, UINT height);
		void _CreateDepthBuffer(UINT width, UINT height);
		void _SetupDebug();
		void _ReportLiveObjects();
		void _ReleaseViews();

		bool _OnWindowResize(WindowResizeEvent& e);
		bool _OnDebugOptionChanged(DebugOptionChangedEvent& e);

		Window* _window;
		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain{};
		Microsoft::WRL::ComPtr<ID3D11Device> _device{};
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> _immediateContext{};
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _rasterizerState{};
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _wireframeRasterizerState{};
		Microsoft::WRL::ComPtr<ID3D11Texture2D> _depthTexture{};
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthStencilView{};
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _renderTargetView{};

		bool _isWireframeEnabled = false;

		friend class Renderer;
	};
}

