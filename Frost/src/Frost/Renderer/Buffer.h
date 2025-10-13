#pragma once

#include <wrl/client.h>
#include <d3d11.h>

namespace Frost
{
	class Buffer
	{
	public:
		virtual ID3D11Buffer* Get() { return _buffer.Get(); }
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
		void Create(const void* data, UINT dataSize, UINT dataCount);
		void Bind();
		UINT GetCount() const { return _count; }

	private:
		UINT _count{};
	};

	class ConstantBuffer : public Buffer
	{
	public:
		void Create(const void* data, UINT dataSize);
		void Bind();
	};
}