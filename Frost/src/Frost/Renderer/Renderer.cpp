#include "Renderer.h"
#include "Frost/Core/Application.h"

#include <cassert>

namespace Frost
{
	class CreateBufferFailed {};
	class CreateVertexShaderFailed {};
	class CreateInputLayoutFailed {};
	class CreatePixelShaderFailed {};

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

	void Renderer::CreateBuffer(const D3D11_BUFFER_DESC* descriptor, const D3D11_SUBRESOURCE_DATA* subresourceData, ID3D11Buffer** buffer)
	{
		HRESULT result = _device->_device->CreateBuffer(descriptor, subresourceData, buffer);
		
		if (FAILED(result))
		{
			MessageBox(NULL, L"Create Buffer Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw CreateBufferFailed{};
		}
	}

	void Renderer::CreateVertexShader(const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11VertexShader** vertexShader)
	{
		HRESULT result = _device->_device->CreateVertexShader(shaderBytecode, bytecodeLength, nullptr, vertexShader);
		if (FAILED(result))
		{
			MessageBox(NULL, L"Create Vertex Shader Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw CreateVertexShaderFailed{};
		}
	}

	void Renderer::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inputElementDescs, UINT numElements, const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11InputLayout** inputLayout)
	{
		HRESULT result = _device->_device->CreateInputLayout(inputElementDescs, numElements, shaderBytecode, bytecodeLength, inputLayout);
		if (FAILED(result))
		{
			MessageBox(NULL, L"Create Input Layout Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw CreateInputLayoutFailed{};
		}
	}

	void Renderer::CreatePixelShader(const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11PixelShader** pixelShader)
	{
		HRESULT result = _device->_device->CreatePixelShader(shaderBytecode, bytecodeLength, nullptr, pixelShader);
		if (FAILED(result))
		{
			MessageBox(NULL, L"Create Pixel Shader Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw CreatePixelShaderFailed{};
		}
	}
}
