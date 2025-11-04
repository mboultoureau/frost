#pragma once

#include "Frost/Renderer/Texture.h"

#include <d3d11.h>
#include <assimp/types.h>
#include <string>

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

	private:
		ID3D11ShaderResourceView* _textureView = nullptr;
		void CreateD3D11TextureView(int width, int height, const void* data, DXGI_FORMAT format);
	};
}