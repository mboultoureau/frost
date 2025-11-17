#include "BufferDX11.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Renderer/DX11/CommandListDX11.h"

namespace Frost
{
	BufferDX11::BufferDX11(const BufferConfig& config, ID3D11Device* device, const void* initialData)
		: _config(config)
	{
		FT_ENGINE_ASSERT(device, "D3D11Device cannot be null for buffer creation!");

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = config.size;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		if (config.dynamic)
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
		}

		switch (config.usage)
		{
		case BufferUsage::VERTEX_BUFFER:
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case BufferUsage::INDEX_BUFFER:
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case BufferUsage::CONSTANT_BUFFER:
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			// Constants buffers must be multiple of 16
			desc.ByteWidth = (config.size + 15) & ~15;
			break;
		default:
			FT_ENGINE_ASSERT(false, "Unknown buffer usage!");
			return;
		}

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		D3D11_SUBRESOURCE_DATA* pInitialData = nullptr;
		if (initialData)
		{
			subresourceData.pSysMem = initialData;
			pInitialData = &subresourceData;
		}

		HRESULT hr = device->CreateBuffer(&desc, pInitialData, _buffer.GetAddressOf());
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 buffer!");
	}

	const BufferConfig& BufferDX11::GetConfig() const
	{
		return _config;
	}

	void BufferDX11::UpdateData(CommandList* commandList, const void* data, uint32_t size, uint32_t offset)
	{
		FT_ENGINE_ASSERT(_config.dynamic, "Cannot update a static buffer!");
		FT_ENGINE_ASSERT(size + offset <= _config.size, "Update data is out of bounds!");
		FT_ENGINE_ASSERT(commandList, "CommandList cannot be null for buffer update!");

		CommandListDX11* dx11CommandList = static_cast<CommandListDX11*>(commandList);
		FT_ENGINE_ASSERT(dx11CommandList, "Provided CommandList is not a CommandListDX11!");

		ID3D11DeviceContext* context = static_cast<ID3D11DeviceContext*>(dx11CommandList->GetNativeRenderContext());
		FT_ENGINE_ASSERT(context, "Failed to get D3D11DeviceContext from CommandListDX11!");


		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT hr = context->Map(_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to map dynamic buffer for update!");

		uint8_t* dest = reinterpret_cast<uint8_t*>(mappedResource.pData) + offset;
		memcpy(dest, data, size);

		context->Unmap(_buffer.Get(), 0);
	}

	uint32_t BufferDX11::GetSize() const
	{
		return _config.size;
	}

	ID3D11Buffer* BufferDX11::GetD3D11Buffer() const
	{
		return _buffer.Get();
	}
}