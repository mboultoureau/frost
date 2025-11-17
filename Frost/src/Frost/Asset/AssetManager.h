#pragma once

#include "Frost/Asset/Asset.h"
#include "Frost/Asset/Texture.h"

#ifdef FT_PLATFORM_WINDOWS
	#include "Frost/Renderer/DX11/TextureDX11.h"
#endif

#include <memory>
#include <map>

namespace Frost
{
	class AssetManager
	{
	public:


		// Generic asset loading function
		template<typename T, typename... Args>
		static std::shared_ptr<T> LoadAsset(const Asset::Path& path, Args&&... args) requires (!std::is_same_v<T, Texture>)
		{
			static_assert(std::is_base_of<Asset, T>::value, "T must be derived from Frost::Asset");

			static std::map<Asset::Path, std::shared_ptr<Asset>> assetCache;

			// Check if asset is already loaded
			auto it = assetCache.find(path);
			if (it != assetCache.end())
			{
				return std::static_pointer_cast<T>(it->second);
			}

			// Load new asset
			std::shared_ptr<T> asset = std::make_shared<T>(std::forward<Args>(args)...);
			assetCache[path] = asset;
			return asset;
		}

		// Specialized function to get already loaded asset
		static std::shared_ptr<Texture> LoadAsset(const Asset::Path& path, TextureConfig& config)
		{
			static std::map<Asset::Path, std::shared_ptr<Asset>> assetCache;

			auto it = assetCache.find(path);
			if (it != assetCache.end())
			{
				return std::static_pointer_cast<Texture>(it->second);
			}

#ifdef FT_PLATFORM_WINDOWS
			std::shared_ptr<TextureDX11> texture = std::make_shared<TextureDX11>(config);
#else
#error "Texture loading not implemented for this platform"
#endif
			assetCache[path] = texture;
			return texture;
		}
	};
}


