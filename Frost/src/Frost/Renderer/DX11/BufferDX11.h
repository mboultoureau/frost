#pragma once

#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/CommandList.h"

#include <d3d11.h>
#include <wrl/client.h>

namespace Frost
{
	class BufferDX11 : public Buffer
	{
	public:
		BufferDX11(const BufferConfig& config, ID3D11Device* device, const void* initialData = nullptr);
		virtual ~BufferDX11() override = default;

		virtual const BufferConfig& GetConfig() const override;
		virtual void UpdateData(CommandList* commandList, const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual uint32_t GetSize() const override;

		ID3D11Buffer* GetD3D11Buffer() const;

	private:
		BufferConfig _config;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _buffer;
	};
}