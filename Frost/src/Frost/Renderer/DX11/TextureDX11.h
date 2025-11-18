#pragma once

#include "Frost/Asset/Texture.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <array>

struct aiTexture;

namespace Frost
{
	class TextureDX11 : public Texture
	{
	public:
		TextureDX11(TextureConfig& config);
		TextureDX11(ID3D11Texture2D* existingTexture, ID3D11RenderTargetView* rtv);
		TextureDX11(ID3D11Texture2D* existingTexture, ID3D11DepthStencilView* dsv);
		TextureDX11(uint32_t width, uint32_t height, Format format, const void* pixelData, const std::string& debugName = "TextureFromData");
		virtual ~TextureDX11() override = default;

		virtual const std::vector<uint8_t> GetData() const override;
		virtual void Bind(Slot slot) const override;

		ID3D11ShaderResourceView* GetShaderResourceView() const { return _srv.Get(); }
		ID3D11RenderTargetView* GetRenderTargetView() const { return _rtv.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return _dsv.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _rtv;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _dsv;

	private:
		void _CreateCubemap(TextureConfig& config);
	};
}