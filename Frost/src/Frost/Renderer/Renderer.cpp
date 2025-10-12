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

	void Renderer::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
	{
		assert(_device != nullptr);
		_device->_immediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void Renderer::Present()
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

	void Renderer::SetViewport(const Viewport& viewport)
	{
		assert(_device != nullptr);

		D3D11_VIEWPORT vp;
		vp.TopLeftX = viewport.x * Application::Get().GetWindow()->GetDimensions().width;
		vp.TopLeftY = viewport.y * Application::Get().GetWindow()->GetDimensions().height;
		vp.Width = viewport.width * Application::Get().GetWindow()->GetDimensions().width;
		vp.Height = viewport.height * Application::Get().GetWindow()->GetDimensions().height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		_device->_immediateContext->RSSetViewports(1, &vp);
		_device->_immediateContext->OMSetRenderTargets(1, _device->_renderTargetView.GetAddressOf(), _device->_depthStencilView.Get());
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

	void Renderer::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		assert(_device != nullptr);
		_device->_immediateContext->IASetPrimitiveTopology(topology);
	}

	void Renderer::SetVertexBuffer(VertexBuffer& buffer, UINT stride, UINT offset)
	{
		assert(_device != nullptr);
		ID3D11Buffer* vertexBuffer = buffer.Get();
		_device->_immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	}

	void Renderer::SetIndexBuffer(IndexBuffer& buffer, UINT offset)
	{
		assert(_device != nullptr);
		ID3D11Buffer* indexBuffer = buffer.Get();
		_device->_immediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, offset);
	}

	void Renderer::SetInputLayout(ID3D11InputLayout* inputLayout)
	{
		assert(_device != nullptr);
		_device->_immediateContext->IASetInputLayout(inputLayout);
	}

	void Renderer::EnableVertexShader(VertexShader& vertexShader)
	{
		assert(_device != nullptr);
		_device->_immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
	}

	void Renderer::EnablePixelShader(PixelShader& pixelShader)
	{
		assert(_device != nullptr);
		_device->_immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	void Renderer::UpdateSubresource(ID3D11Buffer* buffer, const void* data, UINT dataSize)
	{
		assert(_device != nullptr);
		_device->_immediateContext->UpdateSubresource(buffer, 0, nullptr, data, 0, 0);
	}

	void Renderer::SetVertexConstantBuffer(UINT slot, ID3D11Buffer* buffer)
	{
		assert(_device != nullptr);
		_device->_immediateContext->VSSetConstantBuffers(slot, 1, &buffer);
	}

	void Renderer::SetPixelConstantBuffer(UINT slot, ID3D11Buffer* buffer)
	{
		assert(_device != nullptr);
		_device->_immediateContext->PSSetConstantBuffers(slot, 1, &buffer);
	}

	void Renderer::SetGeometryShader()
	{
		assert(_device != nullptr);
		_device->_immediateContext->GSSetShader(nullptr, nullptr, 0);
	}
}
