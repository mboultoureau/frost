#pragma once

#include <string>
#include <d3d11.h>

struct aiTexture;

namespace Frost
{
	// Related to assimp texture types
	enum TextureType
	{
		NONE = 0,
		DIFFUSE = 1,
		SPECULAR = 2,
		AMBIENT = 3,
		EMISSIVE = 4,
		HEIGHT = 5,
		NORMALS = 6,
		SHININESS = 7,
		OPACITY = 8,
		DISPLACEMENT = 9,
		LIGHTMAP = 10,
		REFLECTION = 11,

		// PBR
		BASE_COLOR = 12,
		NORMAL_CAMERA = 13,
		EMISSION_COLOR = 14,
		METALNESS = 15,
		DIFFUSE_ROUGHNESS = 16,
		AMBIENT_OCCLUSION = 17,

		UNKNOWN = 18,
		SHEEN = 19,
		CLEARCOAT = 20,
		TRANSMISSION = 21,
	}; 

	class Texture
	{
	public:
		Texture(const std::string& path, const TextureType textureType);
		Texture(const aiTexture* aTexture, const TextureType textureType);
		~Texture();

		const std::string& GetPath() const { return _path; }
		const TextureType GetTextureType() const { return _textureType; }
		ID3D11ShaderResourceView* GetTextureView() const { return _textureView; }

	private:
		TextureType _textureType;
		std::string _path;
		ID3D11ShaderResourceView* _textureView = nullptr;

		void CreateD3D11TextureView(int width, int height, const void* data, DXGI_FORMAT format);
	};
}


