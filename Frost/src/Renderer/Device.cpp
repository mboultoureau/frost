#include "Device.h"

#include <array>
#include <cassert>

namespace Frost
{
	class WindowPointerExpired{};
	class DeviceCreationFailed{};

	Device::Device(Window* window) : _window{ window }
	{
		assert(_window != nullptr);
		_CreateDevice();
	}

	void Device::_CreateDevice()
	{
		WindowDimensions dimensions = _window->GetDimensions();

		UINT flags = 0x0;
#ifdef FT_DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		DXGI_MODE_DESC bufferDesc = {
			dimensions.width,
			dimensions.height,
			DXGI_RATIONAL { 60, 1 },
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
			DXGI_MODE_SCALING_UNSPECIFIED
		};

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {
			bufferDesc,
			DXGI_SAMPLE_DESC { 1, 0 },
			DXGI_USAGE_RENDER_TARGET_OUTPUT,
			1,
			_window->_hwnd,
			_window->_isWindowed,
			DXGI_SWAP_EFFECT_DISCARD,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		};

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
}