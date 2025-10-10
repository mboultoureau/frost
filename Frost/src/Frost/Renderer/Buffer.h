#pragma once

#include <wrl/client.h>
#include <d3d11.h>

namespace Frost
{
	class VertexBuffer
	{
	public:
		void Bind();

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer;
	};

	class IndexBuffer
	{
	public:
		void Bind();

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer;
	};

}