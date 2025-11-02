#include "Frost/Renderer/TextureLibrary.h"
#include "Frost.h"

namespace Frost
{
	TextureLibrary& TextureLibrary::Get()
	{
		static TextureLibrary instance;
		return instance;
	}

	std::shared_ptr<Texture> TextureLibrary::GetTexture(const std::string& path, const TextureType textureType)
	{
		FT_ENGINE_ASSERT(!path.empty(), "Texture path cannot be empty!");

		// Check if the texture is already loaded
		if (_textures.find(path) != _textures.end())
		{
			return _textures[path];
		}

		// Load the texture
		std::shared_ptr<Texture> texture = std::make_shared<Texture>(path, textureType);
		_textures[path] = texture;
		
		return texture;
	}

	std::shared_ptr<Texture> TextureLibrary::GetTexture(const aiTexture* aTexture, const TextureType textureType)
	{
		FT_ENGINE_ASSERT(aTexture != nullptr, "Assimp texture cannot be null!");

		return std::make_shared<Texture>(aTexture, textureType);
	}
}