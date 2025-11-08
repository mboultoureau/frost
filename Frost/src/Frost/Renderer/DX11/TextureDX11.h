#pragma once

#include "Frost/Renderer/Texture.h"

#include <d3d11.h>
#include <string>
#include <cstdint>
#include <vector>

struct aiTexture;

struct aiTexture;

namespace Frost
{

	class TextureDX11 : public Texture
	{
	public:
		TextureDX11(const std::string& path, const TextureType textureType);
		TextureDX11(const aiTexture* aTexture, const TextureType textureType);
		TextureDX11(const int width, const int height, const TextureType textureType);
		virtual ~TextureDX11() override;

		ID3D11ShaderResourceView* GetTextureView() const { return _textureView; }
		std::vector<uint8_t> GetTextureRawData() const { return _data; }

	private:
		ID3D11ShaderResourceView* _textureView = nullptr;
		void CreateD3D11TextureView(int width, int height, const void* data, DXGI_FORMAT format);
		std::vector<uint8_t> _data;
	};
}