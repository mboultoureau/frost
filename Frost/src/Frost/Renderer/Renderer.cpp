#include "Renderer.h"
#include "Frost/Core/Application.h"

#include <cassert>

namespace Frost
{
	Renderer::Renderer()
	{
		_device = std::make_unique<Device>(Application::Get().GetWindow().get());
	}

	void Renderer::Init()
	{
		assert(_device != nullptr);
	}

	void Renderer::DrawFrame()
	{
		assert(_device != nullptr);
		_device->_swapChain->Present(1, 0);
	}

	void Renderer::ClearColor(float r, float g, float b, float a)
	{
		assert(_device != nullptr);
		float clearColor[4] = { r, g, b, a };
		_device->_immediateContext->ClearRenderTargetView(_device->_renderTargetView.Get(), clearColor);
		_device->_immediateContext->ClearDepthStencilView(_device->_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}
