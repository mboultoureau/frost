#pragma once

#include <wrl/client.h>
#include <d3d11.h>

namespace Frost
{
	class Buffer
	{
	public:
		virtual void Create(const void* data, UINT dataSize) = 0;
		virtual void Bind() = 0;

	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer;
	};

	class VertexBuffer : public Buffer
	{
	public:
		void Create(const void* data, UINT dataSize);
		void Bind();
	};

	class IndexBuffer : public Buffer
	{
	public:
		void Create(const void* data, UINT dataSize);
		void Bind();
	};

	class ConstantBuffer : public Buffer
	{
	public:
		void Create(const void* data, UINT dataSize);
		void Bind();
	};
}