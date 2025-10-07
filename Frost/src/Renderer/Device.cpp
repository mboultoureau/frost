#include "Device.h"

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
		_CreateRenderTargetView();
		_CreateRasterizer();
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
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = _window->_hwnd;
		swapChainDesc.Windowed = _window->_isWindowed;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
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
	}

	void Device::_CreateRenderTargetView()
	{
		ID3D11Texture2D* backBuffer = nullptr;
		HRESULT result = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		if (result != S_OK)
		{
			MessageBox(NULL, L"Failed to get back buffer!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw GetBackBufferFailed{};
		}

		result = _device->CreateRenderTargetView(backBuffer, NULL, &_renderTargetView);
		backBuffer->Release();
		if (result != S_OK)
		{
			MessageBox(NULL, L"Failed to create render target view!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw RenderTargetViewCreationFailed{};
		}

		_CreateDepthBuffer();

		_immediateContext->OMSetRenderTargets(1, &_renderTargetView, NULL);
		_renderTargetView->Release();
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<FLOAT>(_window->GetDimensions().width);
		viewport.Height = static_cast<FLOAT>(_window->GetDimensions().height);
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
		_immediateContext->RSSetState(_rasterizerState);
	}

	void Device::_CreateDepthBuffer()
	{
		D3D11_TEXTURE2D_DESC depthBufferDesc = {};
		depthBufferDesc.Width = _window->GetDimensions().width;
		depthBufferDesc.Height = _window->GetDimensions().height;
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

		result = _device->CreateDepthStencilView(_depthTexture, &depthStencilViewDesc, &_depthStencilView);
		if (result != S_OK)
		{
			MessageBox(NULL, L"Failed to create depth stencil view!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw DepthStencilBufferCreationFailed{};
		}
	}

	void Device::DrawFrame()
	{
		const FLOAT clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
		_immediateContext->ClearRenderTargetView(_renderTargetView, clearColor);
		_immediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		_swapChain->Present(1, 0);
	}
}