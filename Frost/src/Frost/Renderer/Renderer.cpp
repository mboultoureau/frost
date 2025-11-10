#include "Renderer.h"
#include "Frost/Core/Application.h"
#include "Frost.h"

#include <cassert>
#include <d3d11_1.h>

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
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
	}

	void Renderer::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void Renderer::Present()
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_swapChain->Present(1, 0);
	}

	void Renderer::ClearColor(float r, float g, float b, float a)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		float clearColor[4] = { r, g, b, a };
		_device->_immediateContext->ClearRenderTargetView(_device->_renderTargetView.Get(), clearColor);
		_device->_immediateContext->ClearDepthStencilView(_device->_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void Renderer::ClearColor(const Viewport& viewport, float r, float g, float b, float a)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		D3D11_RECT vp;
		vp.left = viewport.x * Application::Get().GetWindow()->GetRenderedZoneDimensions().width;
		vp.top = viewport.y * Application::Get().GetWindow()->GetRenderedZoneDimensions().height;
		vp.right = vp.left + (viewport.width * Application::Get().GetWindow()->GetRenderedZoneDimensions().width);
		vp.bottom = vp.top + (viewport.height * Application::Get().GetWindow()->GetRenderedZoneDimensions().height);

		float clearColor[4] = { r, g, b, a };
		
		_device->_immediateContext->ClearView(_device->_renderTargetView.Get(), clearColor, &vp, 1);
	}

	void Renderer::SetViewport(const Viewport& viewport)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");

		D3D11_VIEWPORT vp;
		vp.TopLeftX = viewport.x * Application::Get().GetWindow()->GetRenderedZoneDimensions().width;
		vp.TopLeftY = viewport.y * Application::Get().GetWindow()->GetRenderedZoneDimensions().height;
		vp.Width = viewport.width * Application::Get().GetWindow()->GetRenderedZoneDimensions().width;
		vp.Height = viewport.height * Application::Get().GetWindow()->GetRenderedZoneDimensions().height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		_device->_immediateContext->RSSetViewports(1, &vp);
		_device->_immediateContext->OMSetRenderTargets(1, _device->_renderTargetView.GetAddressOf(), _device->_depthStencilView.Get());
	}

	void Renderer::CreateBuffer(const D3D11_BUFFER_DESC* descriptor, const D3D11_SUBRESOURCE_DATA* subresourceData, ID3D11Buffer** buffer)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");

		HRESULT result = _device->_device->CreateBuffer(descriptor, subresourceData, buffer);
		
		if (FAILED(result))
		{
			MessageBox(NULL, L"Create Buffer Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw CreateBufferFailed{};
		}
	}

	void Renderer::CreateVertexShader(const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11VertexShader** vertexShader)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");

		HRESULT result = _device->_device->CreateVertexShader(shaderBytecode, bytecodeLength, nullptr, vertexShader);
		if (FAILED(result))
		{
			MessageBox(NULL, L"Create Vertex Shader Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw CreateVertexShaderFailed{};
		}
	}

	void Renderer::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inputElementDescs, UINT numElements, const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11InputLayout** inputLayout)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");

		HRESULT result = _device->_device->CreateInputLayout(inputElementDescs, numElements, shaderBytecode, bytecodeLength, inputLayout);
		if (FAILED(result))
		{
			MessageBox(NULL, L"Create Input Layout Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw CreateInputLayoutFailed{};
		}
	}

	void Renderer::CreatePixelShader(const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11PixelShader** pixelShader)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");

		HRESULT result = _device->_device->CreatePixelShader(shaderBytecode, bytecodeLength, nullptr, pixelShader);
		if (FAILED(result))
		{
			MessageBox(NULL, L"Create Pixel Shader Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			throw CreatePixelShaderFailed{};
		}
	}

	void Renderer::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->IASetPrimitiveTopology(topology);
	}

	void Renderer::SetVertexBuffer(const VertexBuffer& buffer, UINT stride, UINT offset)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		ID3D11Buffer* vertexBuffer = buffer.Get();
		_device->_immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	}

	void Renderer::SetIndexBuffer(const IndexBuffer& buffer, UINT offset)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		ID3D11Buffer* indexBuffer = buffer.Get();
		_device->_immediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, offset);
	}

	void Renderer::SetInputLayout(ID3D11InputLayout* inputLayout)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->IASetInputLayout(inputLayout);
	}

	void Renderer::EnableVertexShader(VertexShader& vertexShader)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
	}

	void Renderer::EnablePixelShader(PixelShader& pixelShader)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	void Renderer::UpdateSubresource(ID3D11Buffer* buffer, const void* data, UINT dataSize)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->UpdateSubresource(buffer, 0, nullptr, data, 0, 0);
	}

	void Renderer::SetVertexConstantBuffer(UINT slot, ID3D11Buffer* buffer)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->VSSetConstantBuffers(slot, 1, &buffer);
	}

	void Renderer::SetPixelConstantBuffer(UINT slot, ID3D11Buffer* buffer)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->PSSetConstantBuffers(slot, 1, &buffer);
	}

	void Renderer::SetGeometryShader()
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->GSSetShader(nullptr, nullptr, 0);
	}

	bool Renderer::CreateTexture2D(const D3D11_TEXTURE2D_DESC* descriptor, const D3D11_SUBRESOURCE_DATA* subresourceData, ID3D11Texture2D** texture)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		HRESULT result = _device->_device->CreateTexture2D(descriptor, subresourceData, texture);
		
		if (FAILED(result))
		{
			FT_ENGINE_ERROR("Failed to create Texture2D");
			return false;
		}

		return true;
	}

	bool Renderer::CreateShaderResourceView(ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc, ID3D11ShaderResourceView** shaderResourceView)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		HRESULT result = _device->_device->CreateShaderResourceView(resource, srvDesc, shaderResourceView);
		
		if (FAILED(result))
		{
			FT_ENGINE_ERROR("Failed to create Shader Resource View");
			return false;
		}
		return true;
	}

	void Renderer::CreateSamplerState(const D3D11_SAMPLER_DESC* samplerDesc, ID3D11SamplerState** samplerState)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		HRESULT result = _device->_device->CreateSamplerState(samplerDesc, samplerState);
		FT_ENGINE_ASSERT(SUCCEEDED(result), "Failed to create Sampler State");
	}

	void Renderer::SetPixelSampler(UINT slot, ID3D11SamplerState* samplerState)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->PSSetSamplers(slot, 1, &samplerState);
	}

	void Renderer::SetPixelShaderResource(UINT slot, ID3D11ShaderResourceView* shaderResourceView)
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		_device->_immediateContext->PSSetShaderResources(slot, 1, &shaderResourceView);
	}

	ID3D11Device* Renderer::Get3DDevice() const
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		return _device->_device.Get();
	}

	ID3D11DeviceContext1* Renderer::GetImmediateContext() const
	{
		FT_ENGINE_ASSERT(_device != nullptr, "Renderer device is null");
		return _device->_immediateContext.Get();
	}
}
