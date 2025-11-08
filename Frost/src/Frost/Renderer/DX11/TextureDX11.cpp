#include "Frost/Renderer/DX11/TextureDX11.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/types.h>
#include <assimp/scene.h>

#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
	TextureDX11::TextureDX11(const std::string& path, const TextureType textureType) : Texture(path, textureType)
	{
		FT_ENGINE_INFO("Loading texture: {}", path);

		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		FT_ENGINE_ASSERT(data, "Failed to load texture: {}", path);
		_data = std::vector<uint8_t>( data, data + width * height * 4 );
		_width = width;
		_height = height;
		CreateD3D11TextureView(width, height, data, DXGI_FORMAT_R8G8B8A8_UNORM);
		stbi_image_free(data);
	}

	TextureDX11::TextureDX11(const aiTexture* aTexture, const TextureType textureType) : Texture(aTexture, textureType)
	{
		FT_ENGINE_INFO("Loading embedded texture");

		// if mHeight == 0, texture is compressed (like PNG, JPG, etc.)
		if (aTexture->mHeight != 0)
		{
			int width = (int)aTexture->mWidth;
			int height = (int)aTexture->mHeight;

			std::vector<unsigned char> swizzledData(width * height * 4);
			aiTexel* texels = aTexture->pcData;

			for (int i = 0; i < width * height; ++i)
			{
				swizzledData[i * 4 + 0] = texels[i].r; // R
				swizzledData[i * 4 + 1] = texels[i].g; // G
				swizzledData[i * 4 + 2] = texels[i].b; // B
				swizzledData[i * 4 + 3] = texels[i].a; // A
			}

			CreateD3D11TextureView(width, height, swizzledData.data(), DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		else
		{
			int width, height, numChannels;

			stbi_uc* data = stbi_load_from_memory(
				reinterpret_cast<const stbi_uc*>(aTexture->pcData),
				(int)aTexture->mWidth,
				&width,
				&height,
				&numChannels,
				STBI_rgb_alpha
			);

			FT_ENGINE_ASSERT(data, "Failed to load embedded texture from memory");
			CreateD3D11TextureView(width, height, data, DXGI_FORMAT_R8G8B8A8_UNORM);
			stbi_image_free(data);
		}
	}

	TextureDX11::TextureDX11(const int width, const int height, const TextureType textureType) : Texture(width, height, textureType)
	{
	}

	TextureDX11::~TextureDX11()
	{
		if (_textureView)
		{
			_textureView->Release();
			_textureView = nullptr;
		}
	}

	void TextureDX11::CreateD3D11TextureView(int width, int height, const void* data, DXGI_FORMAT format)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;
		initData.SysMemPitch = width * 4; // RGBA

		ID3D11Texture2D* texture = nullptr;
		FT_ENGINE_ASSERT(RendererAPI::CreateTexture2D(&textureDesc, &initData, &texture), "Failed to create texture2D");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		FT_ENGINE_ASSERT(RendererAPI::CreateShaderResourceView(texture, &srvDesc, &_textureView), "Failed to create shader resource view for texture");
		texture->Release();
	}
}