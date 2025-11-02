#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
	void RendererAPI::ClearColor(float r, float g, float b, float a)
	{
		Get().ClearColor(r, g, b, a);
	}

	void RendererAPI::ClearColor(const Viewport& viewport, float r, float g, float b, float a)
	{
		Get().ClearColor(viewport, r, g, b, a);
	}

	void RendererAPI::Present()
	{
		Get().Present();
	}

	void RendererAPI::SetViewport(const Viewport& viewport)
	{
		Get().SetViewport(viewport);
	}

	void RendererAPI::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		Get().SetPrimitiveTopology(topology);
	}

	void RendererAPI::SetVertexBuffer(const VertexBuffer& buffer, UINT stride, UINT offset)
	{
		Get().SetVertexBuffer(buffer, stride, offset);
	}

	void RendererAPI::SetIndexBuffer(const IndexBuffer& buffer, UINT offset)
	{
		Get().SetIndexBuffer(buffer, offset);
	}

	void RendererAPI::SetInputLayout(ID3D11InputLayout* inputLayout)
	{
		Get().SetInputLayout(inputLayout);
	}

	void RendererAPI::EnableVertexShader(VertexShader& vertexShader)
	{
		Get().EnableVertexShader(vertexShader);
	}

	void RendererAPI::EnablePixelShader(PixelShader& pixelShader)
	{
		Get().EnablePixelShader(pixelShader);
	}

	void RendererAPI::UpdateSubresource(ID3D11Buffer* buffer, const void* data, UINT dataSize)
	{
		Get().UpdateSubresource(buffer, data, dataSize);
	}

	void RendererAPI::SetVertexConstantBuffer(UINT slot, ID3D11Buffer* buffer)
	{
		Get().SetVertexConstantBuffer(slot, buffer);
	}

	void RendererAPI::SetPixelConstantBuffer(UINT slot, ID3D11Buffer* buffer)
	{
		Get().SetPixelConstantBuffer(slot, buffer);
	}

	bool RendererAPI::CreateTexture2D(const D3D11_TEXTURE2D_DESC* descriptor, const D3D11_SUBRESOURCE_DATA* subresourceData, ID3D11Texture2D** texture)
	{
		return Get().CreateTexture2D(descriptor, subresourceData, texture);
	}

	bool RendererAPI::CreateShaderResourceView(ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc, ID3D11ShaderResourceView** shaderResourceView)
	{
		return Get().CreateShaderResourceView(resource, srvDesc, shaderResourceView);
	}

	void RendererAPI::CreateSamplerState(const D3D11_SAMPLER_DESC* samplerDesc, ID3D11SamplerState** samplerState)
	{
		Get().CreateSamplerState(samplerDesc, samplerState);
	}

	void RendererAPI::SetGeometryShader()
	{
		Get().SetGeometryShader();
	}

	void RendererAPI::SetPixelSampler(UINT slot, ID3D11SamplerState* samplerState)
	{
		Get().SetPixelSampler(slot, samplerState);
	}

	void RendererAPI::SetPixelShaderResource(UINT slot, ID3D11ShaderResourceView* shaderResourceView)
	{
		Get().SetPixelShaderResource(slot, shaderResourceView);
	}

	ID3D11Device* RendererAPI::Get3DDevice()
	{
		return Get().Get3DDevice();
	}

	ID3D11DeviceContext1* RendererAPI::GetImmediateContext()
	{
		return Get().GetImmediateContext();
	}

	Renderer& RendererAPI::Get()
	{
		static Renderer _renderer;
		return _renderer;
	}

	void RendererAPI::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
	{
		Get().DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}
}