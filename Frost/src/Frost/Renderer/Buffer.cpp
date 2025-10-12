#include "Buffer.h"

#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
	void VertexBuffer::Create(const void* data, UINT dataSize)
	{
		D3D11_BUFFER_DESC descriptor{};
		descriptor.Usage = D3D11_USAGE_IMMUTABLE;
		descriptor.ByteWidth = dataSize;
		descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		descriptor.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subresource_data;
		subresource_data.pSysMem = data;

		RendererAPI::Get().CreateBuffer(&descriptor, &subresource_data, &_buffer);
	}

	void VertexBuffer::Bind()
	{

	}

	void IndexBuffer::Create(const void* data, UINT dataSize, UINT dataCount)
	{
		_count = dataCount;

		D3D11_BUFFER_DESC descriptor{};
		descriptor.Usage = D3D11_USAGE_IMMUTABLE;
		descriptor.ByteWidth = dataSize;
		descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
		descriptor.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subresource_data;
		subresource_data.pSysMem = data;

		RendererAPI::Get().CreateBuffer(&descriptor, &subresource_data, &_buffer);
	}

	void IndexBuffer::Bind()
	{

	}

	void ConstantBuffer::Create(const void* data, UINT dataSize)
	{
		D3D11_BUFFER_DESC descriptor{};
		descriptor.Usage = D3D11_USAGE_DEFAULT;
		descriptor.ByteWidth = dataSize;
		descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		descriptor.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subresource_data;
		subresource_data.pSysMem = data;
		
		RendererAPI::Get().CreateBuffer(&descriptor, nullptr, &_buffer);
	}

	void ConstantBuffer::Bind()
	{
	}
}