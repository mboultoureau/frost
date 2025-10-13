#include "Device.h"
#include "Frost/Core/Application.h"
#include "Frost/Event/WindowResizeEvent.h"

#include <array>
#include <cassert>

namespace Frost
{
	class WindowPointerExpired {};
	class DeviceCreationFailed {};
	class GetBackBufferFailed {};
	class RenderTargetViewCreationFailed {};
	class DepthStencilViewCreationFailed {};
	class DepthStencilBufferCreationFailed {};

	Device::Device(Window* window) : _window{ window }
	{
		assert(_window != nullptr);

		_CreateDevice();
		WindowDimensions dimensions = _window->GetDimensions();
		_CreateViewsAndViewport(dimensions.width, dimensions.height);
		_CreateRasterizer();

		Application::Get().GetEventManager().Subscribe<WindowResizeEvent>(
			[&](WindowResizeEvent& e) -> bool
			{
				HandleWindowResize(e.GetWidth(), e.GetHeight());
				return true;
			});
	}

	Device::~Device()
	{
#ifdef FT_DEBUG
		_ReportLiveObjects();
#endif
	}

	void Device::_CreateDevice()
	{
		WindowDimensions dimensions = _window->GetDimensions();

		UINT flags = 0x0;
#ifdef FT_DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		DXGI_MODE_DESC bufferDesc = {};
		bufferDesc.Width = dimensions.width;
		bufferDesc.Height = dimensions.height;
		bufferDesc.RefreshRate = { 60, 1 };
		bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferDesc = bufferDesc;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.OutputWindow = _window->_hwnd;
		swapChainDesc.Windowed = _window->_isWindowed;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		const std::array<D3D_FEATURE_LEVEL, 1> featuresLevels = {
			D3D_FEATURE_LEVEL_11_0
		};

		HRESULT result = D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			flags,
			featuresLevels.data(),
			static_cast<UINT>(featuresLevels.size()),
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&_swapChain,
			&_device,
			NULL,
			&_immediateContext
		);

		if (result != S_OK)
		{
			MessageBox(NULL, L"Device creation failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw DeviceCreationFailed{};
		}

#ifdef FT_DEBUG
		_SetupDebug();
#endif
	}

	void Device::_CreateViewsAndViewport(UINT width, UINT height)
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		HRESULT result = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

		result = _device->CreateRenderTargetView(backBuffer.Get(), NULL, &_renderTargetView);
		if (result != S_OK)
		{
			MessageBox(NULL, L"Failed to create render target view!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw RenderTargetViewCreationFailed{};
		}

		_CreateDepthBuffer(width, height);
		_immediateContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());

		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<FLOAT>(width);
		viewport.Height = static_cast<FLOAT>(height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		_immediateContext->RSSetViewports(1, &viewport);
	}


	void Device::_CreateRasterizer()
	{
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;

		_device->CreateRasterizerState(&rasterizerDesc, &_rasterizerState);
		_immediateContext->RSSetState(_rasterizerState.Get());
	}

	void Device::_CreateDepthBuffer(UINT width, UINT height)
	{
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

		HRESULT result = _device->CreateTexture2D(&depthBufferDesc, NULL, &_depthTexture);

		if (result != S_OK)
		{
			MessageBox(NULL, L"Failed to create depth stencil buffer!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw DepthStencilViewCreationFailed{};
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = depthBufferDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = _device->CreateDepthStencilView(_depthTexture.Get(), &depthStencilViewDesc, &_depthStencilView);
		if (result != S_OK)
		{
			MessageBox(NULL, L"Failed to create depth stencil view!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw DepthStencilBufferCreationFailed{};
		}
	}

	void Device::_SetupDebug()
	{
		ID3D11Debug* d3dDebug = nullptr;
		HRESULT hr = _device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);

		if (SUCCEEDED(hr))
		{
			ID3D11InfoQueue* d3dInfoQueue = nullptr;
			hr = d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue);

			if (SUCCEEDED(hr))
			{
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
				// d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);

				std::array<D3D11_MESSAGE_ID, 0> hide = {
					// D3D11_MESSAGE_ID_DEVICE_CREATE_INFO,
					// D3D11_MESSAGE_ID_IDXGISwapChain_Present_BlitModel
				};
				D3D11_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = static_cast<UINT>(hide.size());
				filter.DenyList.pIDList = hide.data();
				d3dInfoQueue->AddStorageFilterEntries(&filter);

				d3dInfoQueue->Release();
			}
			d3dDebug->Release();
		}
	}

	void Device::_ReportLiveObjects()
	{
		ID3D11Debug* d3dDebug = nullptr;

		if (_device.Get() && SUCCEEDED(_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
		{
			ID3D11InfoQueue* d3dInfoQueue = nullptr;

			if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
			{
				d3dInfoQueue->ClearStoredMessages();
				d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

				d3dInfoQueue->Release();
			}
			d3dDebug->Release();
		}
	}

	void Device::_ReleaseViews()
	{
		if (_immediateContext)
		{
			_immediateContext->OMSetRenderTargets(0, 0, 0);
		}

		_renderTargetView.Reset();
		_depthStencilView.Reset();
		_depthTexture.Reset();

		if (_immediateContext)
		{
			_immediateContext->ClearState();
		}
	}

	void Device::HandleWindowResize(UINT width, UINT height)
	{
		if (!_swapChain)
			return;

		if (width == 0 || height == 0)
		{
			return;
		}

		_ReleaseViews();

		HRESULT hr = _swapChain->ResizeBuffers(
			0,
			width,
			height,
			DXGI_FORMAT_UNKNOWN,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		);

		if (FAILED(hr))
		{
			MessageBox(NULL, L"Failed to resize swap chain buffers!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw DeviceCreationFailed{}; // Throwing a generic error for now
		}

		_CreateViewsAndViewport(width, height);
	}


}