#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Core/Application.h"
#include "Frost/Core/Windows/WindowWin.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/DX11/BufferDX11.h" 

#include <array>
#include <dxgi1_6.h>

#ifdef FT_DEBUG
#include <d3d11sdklayers.h>
#endif

#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

namespace Frost
{
	RendererDX11::RendererDX11() : Renderer{}
	{
		FT_ENGINE_INFO("RendererDX11: initializing.");

		_CreateDevice();
		_CreateRenderTargets();
		_CreateDepthBuffer();
		_CreateDepthStencilStates();
		_CreateRasterizerStates();
		_CreateBlendStates();

		_immediateContext->RSSetState(_solidRasterizerState.Get());

		// Set viewport
		UINT width = Application::GetWindow()->GetWidth();
		UINT height = Application::GetWindow()->GetHeight();

		D3D11_VIEWPORT vp = {};
		vp.Width = static_cast<FLOAT>(width);
		vp.Height = static_cast<FLOAT>(height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		_immediateContext->RSSetViewports(1, &vp);

		FT_ENGINE_INFO("RendererDX11: initialized.");
	}

	RendererDX11::~RendererDX11()
	{
/**
* @see https://seanmiddleditch.github.io/direct3d-11-debug-api-tricks/
*/
#ifdef FT_DEBUG
		FT_ENGINE_TRACE("Reporting live D3D11 device objects...");

		// Log all resource to see memory leak in GPU
		ComPtr<ID3D11Debug> debugController;
		HRESULT hr = _device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(debugController.GetAddressOf()));
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to get D3D11 debug interface!");

		hr = debugController->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to report live device objects!");
#endif

		FT_ENGINE_INFO("RendererDX11: destroyed.");
	}

	void RendererDX11::OnWindowResize(WindowResizeEvent& resizeEvent)
	{
		UINT newWidth = resizeEvent.GetWidth();
		UINT newHeight = resizeEvent.GetHeight();

		FT_ENGINE_TRACE("Window resized to {}x{}", newWidth, newHeight);

		// Ignore if the window is minimized
		if (newWidth == 0 || newHeight == 0)
		{
			FT_ENGINE_WARN("Ignoring window resize to 0x0 (minimized).");
			return;
		}

		_immediateContext->OMSetRenderTargets(0, nullptr, nullptr);
		_backBufferTexture.reset();
		_depthBufferTexture.reset();
		_backBufferRTV.Reset();
		_depthStencilView.Reset();
		_depthTexture.Reset();

		_immediateContext->ClearState();
		_immediateContext->Flush();

		HRESULT hr = _swapChain->ResizeBuffers(
			2,
			newWidth,
			newHeight,
			DXGI_FORMAT_UNKNOWN,
			0
		);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to resize swap chain buffers!");

		_CreateRenderTargets();
		_CreateDepthBuffer();

		D3D11_VIEWPORT vp = {};
		vp.Width = static_cast<FLOAT>(newWidth);
		vp.Height = static_cast<FLOAT>(newHeight);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		_immediateContext->RSSetViewports(1, &vp);

		FT_ENGINE_TRACE("RendererDX11 resized successfully.");
	}

	void RendererDX11::BeginFrame()
	{
		float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		_immediateContext->ClearRenderTargetView(_backBufferRTV.Get(), clearColor);
		_immediateContext->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		_immediateContext->OMSetRenderTargets(1, _backBufferRTV.GetAddressOf(), _depthStencilView.Get());
		_immediateContext->OMSetDepthStencilState(_depthStateReadWrite.Get(), 1);
		_immediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

	}

	void RendererDX11::RestoreBackBufferRenderTarget()
	{
		_immediateContext->OMSetRenderTargets(1, _backBufferRTV.GetAddressOf(), nullptr);
	}

	void RendererDX11::EndFrame()
	{
		HRESULT hr = _swapChain->Present(1, 0);

		if (hr == DXGI_ERROR_DEVICE_RESET || hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			FT_ENGINE_WARN("Device is lose or reset.", hr);
			return;
		}

		if (hr == DXGI_ERROR_INVALID_CALL)
		{
			FT_ENGINE_WARN("Invalid call when presenting swap chain probably due to ALT+ENTER", hr);
			return;
		}

		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to present swap chain!");
	}

	std::shared_ptr<CommandList> RendererDX11::GetNewCommandList()
	{
		return std::make_shared<CommandListDX11>();
	}

	std::shared_ptr<Buffer> RendererDX11::CreateBuffer(const BufferConfig& config, const void* initialData)
	{
		return std::make_shared<BufferDX11>(config, _device.Get(), initialData);
	}

	void RendererDX11::_CreateDevice()
	{
		// Use DirectX 11.1
		const std::array<D3D_FEATURE_LEVEL, 1> featuresLevels = {
			D3D_FEATURE_LEVEL_11_1
		};

		// In Debug mode, enable the debug layer
		UINT flags = 0x0;
#ifdef FT_DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		UINT width = Application::GetWindow()->GetWidth();
		UINT height = Application::GetWindow()->GetHeight();
		FT_ENGINE_ASSERT(width > 0 && height > 0, "Window width and height must be greater than 0!");

		DXGI_MODE_DESC bufferDesc = {};
		bufferDesc.Width = width;
		bufferDesc.Height = height;
		bufferDesc.RefreshRate.Numerator = 0;
		bufferDesc.RefreshRate.Denominator = 0;
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		// Swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc = bufferDesc;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.OutputWindow = static_cast<WindowWin*>(Application::GetWindow())->GetWindowHandle();
		swapChainDesc.Windowed = !static_cast<WindowWin*>(Application::GetWindow())->IsFullscreen();
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = 0;

		ID3D11Device* device = nullptr;

		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			flags,
			featuresLevels.data(),
			static_cast<UINT>(featuresLevels.size()),
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&_swapChain,
			&device,
			NULL,
			&_immediateContext
		);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 device and swap chain!");

		hr = device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(_device.GetAddressOf()));
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to get ID3D11Device1 interface!");

#ifdef FT_DEBUG
		ComPtr<ID3D11InfoQueue> pInfoQueue;
		hr = _device.As(&pInfoQueue);
		if (SUCCEEDED(hr))
		{
			// Break on warning, error, and corruption messages
			pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
			// pInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);

			// Filter messages
			std::array<D3D11_MESSAGE_ID, 0> hideMessages =
			{
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = hideMessages.size();
			filter.DenyList.pIDList = hideMessages.data();

			pInfoQueue->AddStorageFilterEntries(&filter);
		}
#endif
	}

	// For now, we only create the back buffer render target
	void RendererDX11::_CreateRenderTargets()
	{
		// Get back buffer
		ComPtr<ID3D11Texture2D> backBuffer;
		HRESULT hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to get back buffer from swap chain!");

		// Create render target view
		hr = _device->CreateRenderTargetView(backBuffer.Get(), NULL, &_backBufferRTV);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create render target view for back buffer!");

		// Create back buffer texture wrapper
		_backBufferTexture = std::make_unique<TextureDX11>(backBuffer.Get(), _backBufferRTV.Get());
	}

	void RendererDX11::_CreateDepthBuffer()
	{
		UINT width = Application::GetWindow()->GetWidth();
		UINT height = Application::GetWindow()->GetHeight();

		if (width == 0 || height == 0)
		{
			FT_ENGINE_WARN("Skipping depth buffer creation due to zero width or height.");
			return;
		}

		// Create depth buffer texture
		D3D11_TEXTURE2D_DESC depthBufferDesc = {};
		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;

		HRESULT hr = _device->CreateTexture2D(&depthBufferDesc, NULL, _depthTexture.GetAddressOf());
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create depth stencil buffer!");

		// Create depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = depthBufferDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		hr = _device->CreateDepthStencilView(_depthTexture.Get(), &depthStencilViewDesc, &_depthStencilView);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create depth stencil view!");

		_depthBufferTexture = std::make_unique<TextureDX11>(_depthTexture.Get(), _depthStencilView.Get());
	}

	void RendererDX11::_CreateDepthStencilStates()
	{
		HRESULT hr;
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depthStencilDesc.StencilEnable = FALSE;
		hr = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStateReadWrite);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create Read/Write depth stencil state!");

		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		hr = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStateReadOnly);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create ReadOnly depth stencil state!");

		depthStencilDesc.DepthEnable = FALSE;
		hr = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStateNone);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create None depth stencil state!");
	}


	void RendererDX11::_CreateRasterizerStates()
	{
		HRESULT hr;
		D3D11_RASTERIZER_DESC rasterDesc = {};

		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.ScissorEnable = FALSE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;

		hr = _device->CreateRasterizerState(&rasterDesc, &_solidRasterizerState);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create solid rasterizer state!");

		rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
		rasterDesc.CullMode = D3D11_CULL_NONE;

		hr = _device->CreateRasterizerState(&rasterDesc, &_wireframeRasterizerState);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create wireframe rasterizer state!");

		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		hr = _device->CreateRasterizerState(&rasterDesc, &_cullNoneRasterizerState);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create cull-none rasterizer state!");
	}

	void RendererDX11::_CreateBlendStates()
	{
		HRESULT hr;
		D3D11_BLEND_DESC blendDesc = {};

		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		hr = _device->CreateBlendState(&blendDesc, &_blendStateAlpha);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create alpha blend state!");
	}

	ID3D11BlendState* RendererDX11::GetBlendState(BlendMode mode) const
	{
		switch (mode)
		{
			case BlendMode::Alpha: return _blendStateAlpha.Get();
			case BlendMode::None:
			default: return nullptr;
		}
	}

	ID3D11DepthStencilState* RendererDX11::GetDepthStencilState(DepthMode mode) const
	{
		switch (mode)
		{
		case DepthMode::ReadOnly: return _depthStateReadOnly.Get();
		case DepthMode::None: return _depthStateNone.Get();
		case DepthMode::ReadWrite:
		default: return _depthStateReadWrite.Get();
		}
	}
}