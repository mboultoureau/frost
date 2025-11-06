#pragma once

#include "Frost/Renderer/Texture.h"
#include "Frost/Utils/NoCopy.h"
#include "Frost/Renderer/DX11/TextureDX11.h"

#include <assimp/scene.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace Frost
{
	class TextureLibrary : NoCopy
	{
	public:
		static TextureLibrary& Get();
		std::shared_ptr<Texture> GetTexture(const std::string& path, const TextureType textureType);
		std::shared_ptr<Texture> GetTexture(const aiTexture* aTexture, const TextureType textureType);

	private:
		std::unordered_map<std::string, std::shared_ptr<TextureDX11>> _textures;
	};
}

