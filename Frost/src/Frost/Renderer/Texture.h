#pragma once

#include <string>

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


		// DirectX specific
		SHADOW = 100,
	}; 

	class Texture
	{
	public:
		Texture(const std::string& path, const TextureType textureType) {};
		Texture(const aiTexture* aTexture, const TextureType textureType) {};
		Texture(const int width, const int height, const TextureType textureType) {};
		virtual ~Texture() = default;

		const std::string& GetPath() const { return _path; }
		const TextureType GetTextureType() const { return _textureType; }

	protected:
		TextureType _textureType;
		std::string _path;

		int _width;
		int _height;
	};
}


